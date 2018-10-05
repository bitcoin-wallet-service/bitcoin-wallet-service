// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOI_WALLET_SERVICE_SHUTDOWN_H
#define BITCOI_WALLET_SERVICE_SHUTDOWN_H

#include <string>

#include <univalue.h>

enum class RequestExecuteState {
    OK,
    Timeout,
    Disconnected,
    NetworkError,
};

class WalletServiceRequest
{
private:
    UniValue m_data;

public:
    bool setContent(const std::string& data);
    RequestExecuteState execute();
    std::string getResponse();
};


#endif // BITCOI_WALLET_SERVICE_SHUTDOWN_H
