// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_WALLET_SERVICE_RPCBRIDGE_H
#define BITCOIN_WALLET_SERVICE_RPCBRIDGE_H

#include <stdint.h>
#include <string>
#include <vector>

#include <core/uint256.h>
#include <rpcclient.h>

#include <univalue.h>

class BitcoinCoreBridge
{
public:
    BitcoinCoreBridge() {}

    RPCJSONResponse addScript(const uint256 walletID, const std::string &script_hex, const std::string &redeem_script, const std::string &pubkey_hex);
    RPCJSONResponse createWallet(const std::string &wallet_name);
    RPCJSONResponse loadWallet(const std::string &wallet_name);
    void listWallet(const std::string &wallet_name, std::vector<std::string> &list_out);
    RPCJSONResponse createTransaction(const uint256 walletID, const std::string &to_address, int64_t amount, const std::string &change_address, const int conf_target = 2);
};

extern std::unique_ptr<BitcoinCoreBridge> g_bitcoin_core_rpc;

#endif // BITCOIN_WALLET_SERVICE_RPCBRIDGE_H
