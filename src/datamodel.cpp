// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <datamodel.h>

std::unique_ptr<AuthManager> g_auth_manager = std::unique_ptr<AuthManager>(new AuthManager());

void AuthManager::mock() {
    CPubKey pubkey;
    auto pubkey_bin = ParseHex("03eb05b08bd617fa7be13a5dd58792a1df4e1c6782868f7946684ae0cec1604599"); //WIF: L41S4z9RoPoCrcdrUm55Ag8wwebMDGg1d2cL2M7sZYUfD4VrP3NV
    pubkey.Set(pubkey_bin.begin(), pubkey_bin.end());
    AuthUserRef user = addUser(pubkey, uint256(ParseHex("ffff05b08bd617fa7be13a5dd58792a1df4e1c6782868f7946684ae0cec16040")));

    uint256 walletID(ParseHex("ffff05b08bd617faaaa13a5dd58792a1df4e1c6782868f7946684ae0cec16040"));
    WalletRef wallet = user->addWallet(walletID);

    CPubKey pubkey_0;
    auto pubkey0_bin = ParseHex("027dbe32bdf80ef2312f8280bd37bf69c31322745af5ebded6294969ff19c20da3"); //bcrt1qlqx4gx4qftlfe94n48ttchsptxqwt7je7tddna
    pubkey_0.Set(pubkey0_bin.begin(), pubkey0_bin.end());

    wallet->AddPubkey(0, pubkey_0, false);
}

AuthUserRef AuthManager::addUser(const CPubKey &pubkey, const uint256 &id) {
    AuthUserRef user = std::make_shared<AuthUser>();
    user->m_pubkey = pubkey;
    user->m_identity = id;
    m_users[id] = user;
    return user;
}

AuthUserRef AuthManager::findUser(const uint256 userid) {
    auto user_it = m_users.find(userid);
    if (user_it != m_users.end()) {
        return user_it->second;
    }
    return nullptr;
}

WalletRef AuthUser::addWallet(const uint256 walletID) {
    WalletRef wallet = std::make_shared<Wallet>();
    wallet->m_wallet_id = walletID;
    m_wallets[walletID] = wallet;
    return wallet;
}

void Wallet::AddPubkey(unsigned int index, const CPubKey &pubkey, bool change) {
    m_pubkeys[index] = pubkey;
}
