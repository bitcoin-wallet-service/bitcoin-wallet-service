// Copyright (c) 2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINCORE_INDEXD_HASH_H
#define BITCOINCORE_INDEXD_HASH_H

#include <utils.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

class Hash256 {
public:
    uint8_t m_data[32];
    Hash256() {
        memset(m_data, 0, sizeof(m_data));
    }
    Hash256(uint8_t *t) {
        memcpy(m_data, t, sizeof(m_data));
    }
    inline std::string GetHex() const
    {
        return HexStr(std::reverse_iterator<const uint8_t*>(m_data + sizeof(m_data)), std::reverse_iterator<const uint8_t*>(m_data));
    }
    inline int Compare(const Hash256& other) const { return memcmp(m_data, other.m_data, sizeof(m_data)); }
    friend inline bool operator==(const Hash256& a, const Hash256& b) { return a.Compare(b) == 0; }
    friend inline bool operator!=(const Hash256& a, const Hash256& b) { return a.Compare(b) != 0; }
    friend inline bool operator<(const Hash256& a, const Hash256& b) { return a.Compare(b) < 0; }
};

#endif // BITCOINCORE_INDEXD_HASH_H
