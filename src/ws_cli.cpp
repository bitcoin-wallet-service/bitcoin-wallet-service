// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <core/utils.h>
#include "fcgio.h"
#include <univalue.h>
#include <requesthandler.h>

#include <core/key.h>
#include <core/random.h>

#include <bitcoinrpcbridge.h>
#include <datamodel.h>

int main(int argc, char* argv[])
{
    RandomInit();
    ECC_Start();

    // parse arguments
    g_args.ParseParameters(argc, argv);
    InitLogging();

    // create datadir if required
    if (!isDir(GetDataDir())) {
        fprintf(stderr, "Error: Specified data directory \"%s\" does not exist.\n", g_args.GetArg("-datadir", "").c_str());
        exit(1);
    }
    CreateDir(GetDataDir());

    g_auth_manager->mock();
    AuthUserRef us = g_auth_manager->findUser(uint256(ParseHex("ffff05b08bd617fa7be13a5dd58792a1df4e1c6782868f7946684ae0cec16040")));

    RPCJSONResponse  response;
    uint256 walletID(ParseHex("ffff05b08bd617faaaa13a5dd58792a1df4e1c6782868f7946684ae0cec16040"));
    response = g_bitcoin_core_rpc->createWallet(walletID.ToString());
    us->addWallet(walletID);
    std::cout << response.m_response.write(2);

    response = g_bitcoin_core_rpc->addScript(walletID, "0014f80d541aa04afe9c96b3a9d6bc5e015980e5fa59", "", "027dbe32bdf80ef2312f8280bd37bf69c31322745af5ebded6294969ff19c20da3");
    std::cout << response.m_response.write(2);
    response = g_bitcoin_core_rpc->createTransaction(walletID, "mhb6ZbSC5gtjfgxMQEARF1fG1RdS2woNWu", 99000000, "bcrt1qlqx4gx4qftlfe94n48ttchsptxqwt7je7tddna");
    std::cout << response.m_response.write(2);

    response  = CallJSONRPC(walletID.ToString(), "getbalance", UniValue());
    std::cout << response.m_response.write(2);
    WalletServiceRequest request;
    if (!request.setContent(argv[argc-1])) {
        std::cerr << "Invalid request data";
    }
    request.execute();
    std::cout << request.getResponse() << "\n";
}
