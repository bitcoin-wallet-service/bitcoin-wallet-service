// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINCORE_INDEXD_DBINTERFACE_H
#define BITCOINCORE_INDEXD_DBINTERFACE_H

#include <stdint.h>
#include <string>
#include <vector>

#include <hash.h>

//! Interface for the database
class IndexDatabaseInterface
{
public:
    virtual ~IndexDatabaseInterface() {}

    virtual bool close() = 0;

    // loads the blockmap table (maps internal-blockhash-key to blockhash)
    virtual bool loadBlockMap(std::map<unsigned int, Hash256>& blockhash_map, std::map<Hash256, unsigned int>& blockhash_map_rev, unsigned int &counter) = 0;

    // writes an tx index entry
    virtual bool putTxIndex(const uint8_t* key, unsigned int key_len, const uint8_t* value, unsigned int value_len, bool avoid_flush = false) = 0;

    // writes an blockmap-key entry
    virtual bool putBlockMap(const uint8_t* key, unsigned int key_len, const uint8_t* value, unsigned int value_len) = 0;

    // loopup a txid and writes back its blockhash. Returns true if found or false if not found
    virtual bool lookupTXID(const uint8_t* key, unsigned int key_len, Hash256& blockhash) = 0;

    // flush the database if flush requirements are given
    virtual bool flush(bool force = false) = 0;
};

#endif // BITCOINCORE_INDEXD_DBINTERFACE_H
