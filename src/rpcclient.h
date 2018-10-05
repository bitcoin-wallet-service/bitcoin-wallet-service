// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_WALLET_SERVICE_RPCCLIENT_H
#define BITCOIN_WALLET_SERVICE_RPCCLIENT_H

#include <stdint.h>
#include <string>
#include <vector>

#include <univalue.h>
#include <core/uint256.h>

enum class RPCJSONResult {
    OK,
    Failed
};

struct RPCJSONResponse {
    RPCJSONResult m_result = RPCJSONResult::Failed;
    std::string m_error;
    UniValue m_response;
};

enum HTTPStatusCode
{
    HTTP_OK                    = 200,
    HTTP_BAD_REQUEST           = 400,
    HTTP_UNAUTHORIZED          = 401,
    HTTP_FORBIDDEN             = 403,
    HTTP_NOT_FOUND             = 404,
    HTTP_BAD_METHOD            = 405,
    HTTP_INTERNAL_SERVER_ERROR = 500,
    HTTP_SERVICE_UNAVAILABLE   = 503,
};

RPCJSONResponse CallJSONRPC(const std::string &wallet_name, const std::string &method, const UniValue &params);

#endif // BITCOIN_WALLET_SERVICE_RPCCLIENT_H
