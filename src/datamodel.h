// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_WALLET_SERVICE_PERMISSIONS_H
#define BITCOIN_WALLET_SERVICE_PERMISSIONS_H

#include <core/pubkey.h>

enum class WalletType {
    P2PKH,
    P2SH_P2WPKH,
    P2WPKH
};

class Wallet
{
public:
    WalletType m_type;
    uint256 m_wallet_id;
    uint64_t m_creation_date;
    std::map<unsigned int, CPubKey> m_pubkeys;

    void AddPubkey(unsigned int index, const CPubKey &pubkey, bool change);
    //void CreateTransaction(const std::string &to_address, )
};

typedef std::shared_ptr<Wallet> WalletRef;

class AuthUser
{
public:
    uint256 m_identity;
    std::map<uint256, WalletRef> m_wallets;
    unsigned int m_max_wallets = 0;
    CPubKey m_pubkey;
    AuthUser() {}
    WalletRef addWallet(const uint256 walletID);
};

typedef std::shared_ptr<AuthUser> AuthUserRef;

class AuthManager {
public:
    std::map<uint256, AuthUserRef> m_users;

    void mock();

    AuthUserRef addUser(const CPubKey &pubkey, const uint256 &id);
    AuthUserRef findUser(const uint256 userid);
};

extern std::unique_ptr<AuthManager> g_auth_manager;

#endif // BITCOIN_WALLET_SERVICE_PERMISSIONS_H
