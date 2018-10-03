#include "btcnode.h"

#include <btc/block.h>
#include <btc/net.h>
#include <btc/serialize.h>
#include <btc/tx.h>
#include <btc/utils.h>

#include <shutdown.h>
#include <utils.h>

const static bool DEFAULT_WAIT_FOR_BLOCKS = true;

enum BTC_NODE_TYPE {
    BTC_NODE_TYPE_INDEX,
    BTC_NODE_TYPE_FETCH_TX
};

class BTCNodePriv
{
public:
    btc_node_group* m_group;
    BTCNode *m_node;
    std::pair<Hash256, Hash256> m_txid_to_fetch;
    std::vector<unsigned char> m_txdata;
    int m_node_type = BTC_NODE_TYPE_INDEX;

    bool m_index_addr = false;
    BTCNodePriv(BTCNode *node_in);

    ~BTCNodePriv() {
        btc_node_group_free(m_group);
    }

};

btc_bool parse_cmd(struct btc_node_ *node, btc_p2p_msg_hdr *hdr, struct const_buffer *buf)
{
    (void)(node);
    (void)(hdr);
    (void)(buf);
    return true;
}

void request_headers(btc_node *node)
{
    BTCNode *pnode = (BTCNode *)node->nodegroup->ctx;

    // request next headers
    vector *blocklocators = vector_new(1, NULL);
    vector_add(blocklocators, (void *)pnode->GetRawBestBlockHash());

    cstring *getheader_msg = cstr_new_sz(256);
    btc_p2p_msg_getheaders(blocklocators, NULL, getheader_msg);

    /* create p2p message */
    cstring *p2p_msg = btc_p2p_message_new(node->nodegroup->chainparams->netmagic, BTC_MSG_GETHEADERS, getheader_msg->str, getheader_msg->len);
    cstr_free(getheader_msg, true);

    /* send message */
    btc_node_send(node, p2p_msg);
    node->state |= NODE_HEADERSYNC;

    /* cleanup */
    vector_free(blocklocators, true);
    cstr_free(p2p_msg, true);
}

#define MAX_BLOCKS_TO_REQUEST 200
void request_blocks(btc_node *node)
{
    // get the BTCNode pointer from the flexible context
    BTCNode *pnode = (BTCNode *)node->nodegroup->ctx;

    // from an inv message with 2000 blocks
    cstring* inv_msg_cstr = cstr_new_sz(MAX_BLOCKS_TO_REQUEST*(4+32));
    int cnt = 0;

    // loop through the fetched headers and find blocks to request
    for (HeaderEntry* header : pnode->m_headers) {
        if (header->isRequested()) {
            continue;
        }
        unsigned int block_key = 0;
        if (pnode->isIndexed(header->m_hash, &block_key)) {
            continue;
        }
        //LogPrintf("Request block: %s\n", header->m_hash.GetHex());
        ser_u32(inv_msg_cstr, MSG_BLOCK);
        ser_bytes(inv_msg_cstr, header->m_hash.m_data, BTC_HASH_LENGTH);
        pnode->m_blocks_in_flight[header->m_hash] = header;
        header->setRequested();
        if (++cnt == MAX_BLOCKS_TO_REQUEST) break;
    }

    if (pnode->m_blocks_in_flight.size() == 0) {
        LogPrintf("All blocks have been indexed\n");
        // if we have indexed all blocks, force flush database
        if (pnode->db) pnode->db->flush(true);

        if (!g_args.GetBoolArg("-waitforblocks", DEFAULT_WAIT_FOR_BLOCKS)) {
            // disconnect and quit sync
            btc_node_disconnect(node);
        }
        else {
            LogPrintf("Waiting for new blocks...\n");
        }
    }

    // now add the inv counter in a reverse mode
    cstring* inv_msg_cstr_comp = cstr_new_sz(100+MAX_BLOCKS_TO_REQUEST*(4+32));
    ser_varlen(inv_msg_cstr_comp, cnt);
    cstr_append_cstr(inv_msg_cstr_comp, inv_msg_cstr);
    cstr_free(inv_msg_cstr, true);

    // create p2p message
    cstring *p2p_msg = btc_p2p_message_new(node->nodegroup->chainparams->netmagic, BTC_MSG_GETDATA, inv_msg_cstr_comp->str, inv_msg_cstr_comp->len);
    cstr_free(inv_msg_cstr_comp, true);

    // send message
    btc_node_send(node, p2p_msg);
    node->state |= NODE_HEADERSYNC;

    // cleanup
    cstr_free(p2p_msg, true);
}

void check_shutdown(btc_node *node)
{
    if (isShutdownRequested()) {
        BTCNode *pnode = (BTCNode *)node->nodegroup->ctx;
        LogPrintf("Shutdown requested\n");
        if (pnode->db && pnode->db->flush(!pnode->blockflush)) {
            // only allow shutdown here if we can flush the state
            LogPrintf("DB flush state acceptable (no data to flush or everything flushed)\n");
            btc_node_disconnect(node);
        }
    }
}

static btc_bool btc_timer_callback(btc_node *node, uint64_t *now)
{
    BTCNode *pnode = (BTCNode *)node->nodegroup->ctx;
    if (now && pnode->priv->m_node_type == BTC_NODE_TYPE_FETCH_TX) {
        uint64_t timeoffset = *now - node->time_started_con;
        if (timeoffset > 10) { //10 seconds timeout
            pnode->priv->m_txdata.clear();
            btc_node_disconnect(node);
            return true;
        }
    }
    check_shutdown(node);
    return true;
}

void postcmd(struct btc_node_ *node, btc_p2p_msg_hdr *hdr, struct const_buffer *buf)
{
    BTCNode *pnode = (BTCNode *)node->nodegroup->ctx;
    check_shutdown(node);

    if (strcmp(hdr->command, "notfound") == 0 && pnode->priv->m_node_type == BTC_NODE_TYPE_FETCH_TX)
    {
        pnode->priv->m_txdata.clear();
        btc_node_disconnect(node);
        return;
    }
    if (strcmp(hdr->command, BTC_MSG_BLOCK) == 0)
    {
        btc_block_header header;
        if (!btc_block_header_deserialize(&header, buf)) return;

        uint32_t vsize;
        if (!deser_varlen(&vsize, buf)) return;

        uint8_t hash[32];
        btc_block_header_hash(&header, hash);
        Hash256 blockhash(hash);
        auto it = pnode->m_blocks_in_flight.find(blockhash);
        HeaderEntry *pheader = nullptr;
        if (it != pnode->m_blocks_in_flight.end() && it->second != nullptr) {
            pheader = it->second;
        }
        else {
            // block was not requested, check if we have this block already indexed
            if (pnode->isIndexed(blockhash, nullptr) && pnode->priv->m_node_type == BTC_NODE_TYPE_INDEX) {
                // no need to index, abort at this point
                return;
            }
            if (pnode->priv->m_node_type == BTC_NODE_TYPE_FETCH_TX && blockhash == pnode->priv->m_txid_to_fetch.second) {
                // we want a tx from this block
                for (unsigned int i = 0; i < vsize; i++)
                {
                    // since we don't keep index-position of the tx in the index,
                    // search after txid (needs hashing of all txes)
                    btc_tx *tx = btc_tx_new(); //needs to be on the heap
                    size_t len = 0;
                    if (!btc_tx_deserialize((const unsigned char *)buf->p, buf->len, tx, &len, true) || len == 0) {
                        printf("ERROR\n");
                        //TODO better error handling
                    }
                    uint256 txhash_raw;
                    // hash the tx
                    btc_tx_hash(tx, txhash_raw);
                    if (memcmp(txhash_raw, pnode->priv->m_txid_to_fetch.first.m_data, 32) == 0) {
                        // found the tx
                        unsigned char *end = (unsigned char *)(buf->p)+len;
                        pnode->priv->m_txdata.clear();
                        pnode->priv->m_txdata = std::vector<unsigned char>((unsigned char *)buf->p, end);
                        btc_tx_free(tx);
                        // disconnect and leave the event loop
                        btc_node_disconnect(node);
                        return;
                    }
                    btc_tx_free(tx);
                    buf->p = (unsigned char *)buf->p+len;
                }
                btc_node_disconnect(node);
                return;
            }
        }

        // mark block as indexed
        unsigned int primkey = pnode->addBlockToMap(blockhash);
        // create the serialized txindex key at this point (outside of the loop)
        cstring* blockprimkey = cstr_new_sz(4);
        ser_u32(blockprimkey, primkey);

        // write the counter->blockhash map to db
        for (unsigned int i = 0; i < vsize; i++)
        {
            btc_tx *tx = btc_tx_new(); //needs to be on the heap
            size_t len = 0;
            if (!btc_tx_deserialize((const unsigned char *)buf->p, buf->len, tx, &len, true) || len == 0) {
                printf("ERROR\n");
                //TODO better error handling
            }
            buf->p = (unsigned char *)buf->p+len;

            // create a outputs buffer
            cstring* outputs_buf = NULL;
            if (pnode->priv->m_index_addr) {
                for (unsigned int i = 0; i < tx->vout->len; i++) {
                    btc_tx_out *tx_out = (btc_tx_out *)vector_idx(tx->vout, i);
                    vector *script_pushes = vector_new(1, free);
                    enum btc_tx_out_type type = btc_script_classify(tx_out->script_pubkey, script_pushes);
                    if (type == BTC_TX_SCRIPTHASH && script_pushes->len == 1) {
                        uint160 *hash160 = (uint160 *)vector_idx(script_pushes, 0);
                        if (!outputs_buf) {
                            outputs_buf = cstr_new_cstr(blockprimkey);
                        }
                        ser_bytes(outputs_buf, &type, 1);
                        ser_bytes(outputs_buf, hash160, 20);
                    }
                    if (type == BTC_TX_PUBKEYHASH && script_pushes->len == 1) {
                        uint160 *hash160 = (uint160 *)vector_idx(script_pushes, 0);
                        if (!outputs_buf) {
                            outputs_buf = cstr_new_cstr(blockprimkey);
                        }
                        ser_bytes(outputs_buf, &type, 1);
                        ser_bytes(outputs_buf, hash160, 20);
                    }
                    else if (type == BTC_TX_WITNESS_V0_PUBKEYHASH && script_pushes->len == 1) {
                        uint160 *hash160 = (uint160 *)vector_idx(script_pushes, 0);
                        if (!outputs_buf) {
                            outputs_buf = cstr_new_cstr(blockprimkey);
                        }
                        ser_bytes(outputs_buf, &type, 1);
                        ser_bytes(outputs_buf, hash160, 20);
                    }
                    // TODO: P2WSH
                    vector_free(script_pushes, true);
                }
            }

            uint256 txhash_raw;
            btc_tx_hash(tx, txhash_raw);
            Hash256 txhash(txhash_raw);
            pnode->blockflush = true;
            if (pnode->priv->m_index_addr && outputs_buf) {
                pnode->processTXID(outputs_buf->str,outputs_buf->len, txhash, true);
                cstr_free(outputs_buf, true);
            }
            else {
                pnode->processTXID(blockprimkey->str,blockprimkey->len, txhash, true);
            }
            btc_tx_free(tx);
        }

        pnode->persistBlockKey(blockprimkey->str,blockprimkey->len, blockhash);
        if (pnode->db) pnode->db->flush(); // eventually flush at this point
        if (pheader) pheader->setIndexed();
        LogPrintf("Indexing complete of block %s (idx:%d)\n", blockhash.GetHex(), primkey);
        cstr_free(blockprimkey, true);

        // allow to shutdown at this point since the batch can be written here
        if (isShutdownRequested()) {
            if (pnode->db) pnode->db->flush(true);
            btc_node_disconnect(node);
        }

        if (it != pnode->m_blocks_in_flight.end()) {
            pnode->m_blocks_in_flight.erase(it);
            if (pnode->m_blocks_in_flight.size() == 0) {
                request_blocks(node);
            }
        }
    }

    if (strcmp(hdr->command, BTC_MSG_HEADERS) == 0)
    {
        uint32_t amount_of_headers;
        if (!deser_varlen(&amount_of_headers, buf)) return;
        node->nodegroup->log_write_cb("Got %d headers\n\n", amount_of_headers);

        for (unsigned int i=0;i<amount_of_headers;i++)
        {
            btc_block_header header;
            if (!btc_block_header_deserialize(&header, buf)) return;
            uint8_t hash[32];
            btc_block_header_hash(&header, hash);
            if (!pnode->AddHeader(hash, header.prev_block)) {
                node->nodegroup->log_write_cb("Failed to connect header\n");
            }
            /* skip tx count */
            if (!deser_skip(buf, 1)) {
                node->nodegroup->log_write_cb("Header deserialization (tx count skip) failed (node %d)\n", node->nodeid);
                return;
            }
        }

        if (amount_of_headers == MAX_HEADERS_RESULTS)
        {
            int headerchainsize = pnode->GetHeight();
            printf("Sync blockheaders %d%% done (at height: %d)\n", (int)(100.0 / node->bestknownheight * headerchainsize), headerchainsize);

            // peer sent maximal amount of headers, very likely, there will be more
            request_headers(node);
        }
        else
        {
            LogPrintf("Compleded header sync\n");
            request_blocks(node);
        }
    }

    if (strcmp(hdr->command, BTC_MSG_INV) == 0)
    {
        struct const_buffer original_inv = { buf->p, buf->len };
        uint32_t varlen;
        deser_varlen(&varlen, buf);
        btc_bool contains_block = false;

        node->nodegroup->log_write_cb("Got inv message with %d item(s)\n", varlen);

        // if the inv contains one or more blocks, just request the complete inv (lazy mode)
        // TODO: create a new inv message and only request the blocks
        for (unsigned int i=0;i<varlen;i++)
        {
            uint32_t type;
            deser_u32(&type, buf);
            if (type == MSG_BLOCK || type == MSG_WITNESS_BLOCK) {
                contains_block = true;
            }
            deser_skip(buf, 32);
        }

        if (contains_block)
        {
            /* request the blocks */
            node->nodegroup->log_write_cb("Requesting %d blocks via requested inv message\n", varlen);
            cstring *p2p_msg = btc_p2p_message_new(node->nodegroup->chainparams->netmagic, BTC_MSG_GETDATA, original_inv.p, original_inv.len);
            btc_node_send(node, p2p_msg);
            cstr_free(p2p_msg, true);
        }
    }
}

void node_connection_state_changed(struct btc_node_ *node)
{
    (void)(node);
}

void handshake_done(struct btc_node_ *node)
{
    BTCNode *pnode = (BTCNode *)node->nodegroup->ctx;
    if (pnode->priv->m_node_type == BTC_NODE_TYPE_INDEX) {
        request_headers(node);
    }
    else if (pnode->priv->m_node_type == BTC_NODE_TYPE_FETCH_TX) {
        // getdata that txid

        // create a INV
        cstring* inv_msg_cstr = cstr_new_sz(256);
        btc_p2p_inv_msg inv_msg;
        memset(&inv_msg, 0, sizeof(inv_msg));

        btc_p2p_msg_inv_init(&inv_msg, MSG_WITNESS_BLOCK, pnode->priv->m_txid_to_fetch.second.m_data);

        /* serialize the inv count (1) */
        ser_varlen(inv_msg_cstr, 1);
        btc_p2p_msg_inv_ser(&inv_msg, inv_msg_cstr);

        /* request the blocks */
        cstring *p2p_msg = btc_p2p_message_new(node->nodegroup->chainparams->netmagic, BTC_MSG_GETDATA, inv_msg_cstr->str, inv_msg_cstr->len);
        btc_node_send(node, p2p_msg);
        cstr_free(p2p_msg, true);
        cstr_free(inv_msg_cstr, true);

    }
}

BTCNodePriv::BTCNodePriv(BTCNode *node_in) : m_node(node_in), m_node_type(BTC_NODE_TYPE_INDEX) {
    m_group = btc_node_group_new(NULL);
    m_group->desired_amount_connected_nodes = 1;

    if (g_args.GetBoolArg("-netdebug", false)) {
        m_group->log_write_cb = net_write_log_printf;
    }

    m_index_addr = g_args.GetBoolArg("-indexaddr", false);

    // make the c++ m_node instance available from all callbacks (via flexible ctx pointer)
    m_group->ctx = m_node;

    // wire libbtc node callback functions (based on libevent)
    m_group->parse_cmd_cb = parse_cmd;
    m_group->postcmd_cb = postcmd;
    m_group->node_connection_state_changed_cb = node_connection_state_changed;
    m_group->handshake_done_cb = handshake_done;
    m_group->periodic_timer_cb = btc_timer_callback; //connect function pointer for periodic shutdown checks

    // push in genesis hash
    m_node->AddHeader((uint8_t*)&m_group->chainparams->genesisblockhash, NULL);
}

BTCNode::BTCNode(IndexDatabaseInterface *db_in) : db(db_in), priv(new BTCNodePriv(this)) {
    if (db) db->loadBlockMap(m_intcounter_to_hash_map, m_hash_to_intcounter_map, auto_inc_counter);
    btc_node *node = btc_node_new();
    btc_node_set_ipport(node, "127.0.0.1:8333");
    btc_node_group_add_node(priv->m_group, node);
}

BTCNode::~BTCNode() {
    for (HeaderEntry*e : m_headers) {
        delete e;
    }
    m_headers.clear();
    delete priv;
}


void BTCNode::SyncLoop() {
    btc_node_group_connect_next_nodes(priv->m_group);
    btc_node_group_event_loop(priv->m_group);
}

bool BTCNode::FetchTX(const Hash256& tx, const Hash256& block, std::vector<unsigned char> &txdata_out) {
    priv->m_txid_to_fetch = std::make_pair(tx, block);
    priv->m_node_type = BTC_NODE_TYPE_FETCH_TX;
    btc_node_group_connect_next_nodes(priv->m_group);
    btc_node_group_event_loop(priv->m_group);
    if (priv->m_txdata.empty()) {
        return false;
    }
    txdata_out = priv->m_txdata;
    return true;
}

void BTCNode::Loop() {
    btc_node_group_connect_next_nodes(priv->m_group);
    btc_node_group_event_loop(priv->m_group);
}

bool BTCNode::AddHeader(uint8_t* t, uint8_t* prevhash) {
    if (m_headers.size() > 0 && m_headers.back()->m_hash != prevhash) {
        // if we hit this (non sequential headers found), re-bootstrap all headers since this seems to be the safes solution
        LogPrintf("Failed to connect header");
        return false;
    }

    // to avoid storing a 32byte blockhash for each txindex entry, we use a mapping table via an int32 counter
    // the counter is a local mapping and not usefull accross systems
    // we use a upcounting (auto-increment) uint32 primkey
    HeaderEntry *hEntry = new HeaderEntry(t, 0); //add header with primkey to next index
    if (m_blocks.find(hEntry->m_hash) == m_blocks.end()) {
        //not yet in block map
        m_headers.push_back(hEntry);
        m_blocks[m_headers.back()->m_hash] = hEntry;
    }
    return true;
}

void BTCNode::processTXID(void *block_prim_key, uint8_t block_prim_key_len, const Hash256& tx, bool avoid_flush) {
    blockflush = true;
    if (db) db->putTxIndex(tx.m_data, 32, (const uint8_t *)block_prim_key, block_prim_key_len);
}

unsigned int BTCNode::addBlockToMap(const Hash256& hash) {
    m_intcounter_to_hash_map[++auto_inc_counter] = hash;
    m_hash_to_intcounter_map[hash] = auto_inc_counter;
    return auto_inc_counter;
}

void BTCNode::persistBlockKey(void *block_prim_key, uint8_t block_prim_key_len, const Hash256& blockhash) {
    if (db) db->putBlockMap((const uint8_t *)block_prim_key, block_prim_key_len, blockhash.m_data, 32);
    blockflush = false;
}

bool BTCNode::isIndexed(const Hash256& hash, unsigned int *block_prim_key) {
    // if we have the block hash in m_intcounter_to_hash_map, it's indexed;
    auto it = m_hash_to_intcounter_map.find(hash);
    if (it != m_hash_to_intcounter_map.end()) {
        if (block_prim_key) *block_prim_key = it->second;
        return true;
    }
    return false;
}
