// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "requesthandler.h"

#include <univalue.h>

bool WalletServiceRequest::setContent(const std::string& data) {
    if (m_data.read(data) && m_data.isObject()) return true;
    return false;
}

RequestExecuteState WalletServiceRequest::execute() {
    return RequestExecuteState::OK;
}

std::string WalletServiceRequest::getResponse() {
    return m_data.write();
}
