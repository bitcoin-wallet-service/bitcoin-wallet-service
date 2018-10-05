// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bitcoinrpcbridge.h>

#include <chrono>
#include <memory>
#include <stdio.h>

#include <core/tinyformat.h>
#include <core/utils.h>

#include <rpcclient.h>

std::unique_ptr<BitcoinCoreBridge> g_bitcoin_core_rpc = std::unique_ptr<BitcoinCoreBridge>(new BitcoinCoreBridge());

RPCJSONResponse BitcoinCoreBridge::addScript(uint256 walletID, const std::string &script_hex, const std::string &redeem_script, const std::string &pubkey_hex) {
    UniValue params(UniValue::VARR);
    UniValue params_keys(UniValue::VARR);
    UniValue key_object(UniValue::VOBJ);
    UniValue objects_object(UniValue::VOBJ);
    key_object.pushKV("scriptPubKey", script_hex);
    if (!redeem_script.empty()) {
        key_object.pushKV("redeemscript", redeem_script);
    }
    key_object.pushKV("timestamp", "now");
    key_object.pushKV("watchonly", true);
    objects_object.pushKV("rescan", false);
    params_keys.push_back(key_object);
    params.push_back(params_keys);
    params.push_back(objects_object);
    RPCJSONResponse response = CallJSONRPC(walletID.ToString(), "importmulti", params);
    if (!response.m_response.isArray() || response.m_response.size() > 0 || response.m_response[0].isObject()) {
        response.m_error = "import failed";
        response.m_result = RPCJSONResult::Failed;
        return response;
    }
    UniValue success_resp = find_value(response.m_response[0], "success");
    if (!success_resp.isBool() || !success_resp.isTrue()) {
        response.m_error = "import failed";
        response.m_result = RPCJSONResult::Failed;
        return response;
    }
    std::cout << response.m_response.write(2);

    UniValue params_ipub(UniValue::VARR);
    params_ipub.push_back(pubkey_hex);
    params_ipub.push_back(""); // label
    params_ipub.push_back(false); //rescan

    return CallJSONRPC(walletID.ToString(), "importpubkey", params_ipub);
}

RPCJSONResponse BitcoinCoreBridge::createWallet(const std::string &wallet_name) {
    UniValue params(UniValue::VARR);
    params.push_back(wallet_name);
    params.push_back(UniValue(true));
    RPCJSONResponse response = CallJSONRPC("", "createwallet", params);
    if (!response.m_response.isArray() || response.m_response.size() > 0 || response.m_response[0].isObject()) {
        response.m_result = RPCJSONResult::Failed;
        return response;
    }
    UniValue name = find_value(response.m_response[0], "name");
    if (!name.isStr() || name.get_str() != wallet_name) {
        response.m_result = RPCJSONResult::Failed;
        return response;
    }
    return response;
}

RPCJSONResponse BitcoinCoreBridge::loadWallet(const std::string &wallet_name) {
    // TODO
}

void BitcoinCoreBridge::listWallet(const std::string &wallet_name, std::vector<std::string> &list_out) {
    // TODO
}

RPCJSONResponse BitcoinCoreBridge::createTransaction(const uint256 walletID, const std::string &to_address, int64_t amount, const std::string &change_address, const int conf_target) {
    // fundrawtransaction 020000000001c09ee6050000000017a91446d96e0da5bcac1802f33d255c8e67dba29364bd8700000000 '{"includeWatching": true, "changeAddress": "mhb6ZbSC5gtjfgxMQEARF1fG1RdS2woNWu"}'

    // createrawtransaction [] '{"out":val}'

    UniValue ct_params(UniValue::VARR);
    ct_params.push_back(UniValue(UniValue::VARR));
    UniValue outputs(UniValue::VOBJ);
    outputs.pushKV(to_address, ValueFromAmount(amount));
    ct_params.push_back(outputs);

    RPCJSONResponse response = CallJSONRPC(walletID.ToString(), "createrawtransaction", ct_params);
    if (response.m_result == RPCJSONResult::Failed || !response.m_response.isStr()) {
        return response;
    }

    UniValue frt_params(UniValue::VARR);
    frt_params.push_back(response.m_response);
    UniValue options(UniValue::VOBJ);
    options.pushKV("includeWatching", true);
    options.pushKV("changeAddress", change_address);
    options.pushKV("conf_target", conf_target);
    frt_params.push_back(options);
    return CallJSONRPC(walletID.ToString(), "fundrawtransaction", frt_params);
}
