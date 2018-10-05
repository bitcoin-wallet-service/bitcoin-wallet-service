# Copyright (c) 2018 Jonas Schnelli
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS)
AM_CXXFLAGS = $(HARDENED_CXXFLAGS) $(ERROR_CXXFLAGS)
AM_CPPFLAGS = $(HARDENED_CPPFLAGS)
EXTRA_LIBRARIES =

LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

bin_PROGRAMS = wallet-service-fcgi wallet-service-cli

WALLET_SERVICE_COMMON_H = \
    bitcoinrpcbridge.h \
    core/arith_uint256.h \
    compat/byteswap.h \
    compat/endian.h \
    compat.h \
    crypto/common.h \
    crypto/hmac_sha256.h \
    crypto/hmac_sha512.h \
    crypto/ripemd160.h \
    crypto/sha256.h \
    crypto/sha512.h \
    events.h \
    core/hash.h \
    core/key.h \
    core/logging.h \
    netaddress.h \
    datamodel.h \
    core/prevector.h \
    core/pubkey.h \
    core/random.h \
    requesthandler.h \
    rpcclient.h \
    shutdown.h \
    support/allocators/secure.h \
    support/cleanse.h \
    support/events.h \
    support/lockedpool.h \
    core/sync.h \
    core/threadsafety.h \
    core/tinyformat.h \
    core/uint256.h \
    core/utils.h
    
WALLET_SERVICE_COMMON_SOURCE = \
    bitcoinrpcbridge.cpp \
    core/arith_uint256.cpp \
    crypto/hmac_sha256.cpp \
    crypto/hmac_sha512.cpp \
    crypto/ripemd160.cpp \
    crypto/sha256.cpp \
    crypto/sha512.cpp \
    core/key.cpp \
    core/logging.cpp \
    netaddress.cpp \
    datamodel.cpp \
    core/pubkey.cpp \
    core/random.cpp \
    requesthandler.cpp \
    rpcclient.cpp \
    shutdown.cpp \
    support/cleanse.cpp \
    support/lockedpool.cpp \
    core/sync.cpp \
    core/uint256.cpp \
    core/utils.cpp \
    $(BITCOIN_WALLET_SERVICE_H)


WALLET_SERVICE_FCGI_SOURCE = \
    ws_fcgi.cpp

WALLET_SERVICE_CLI_SOURCE = \
    ws_cli.cpp

LIBSECP256K1=secp256k1/libsecp256k1.la
$(LIBSECP256K1): $(wildcard secp256k1/src/*.h) $(wildcard secp256k1/src/*.c) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# application binary #
wallet_service_fcgi_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir) -I$(srcdir)/secp256k1/include $(UNIVALUE_CFLAGS) $(LIBUNIVALUE_CFLAGS)
wallet_service_fcgi_CXXFLAGS = $(AM_CXXFLAGS)
wallet_service_fcgi_SOURCES = $(WALLET_SERVICE_COMMON_SOURCE) $(WALLET_SERVICE_FCGI_SOURCE)
wallet_service_fcgi_LDADD = $(LIBUNIVALUE) $(EVENT_LIBS) $(EVENT_PTHREADS_LIBS) $(FCGI_LIBS) $(SQLITE_LIBS) $(LIBSECP256K1)
wallet_service_fcgi_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)
wallet_service_fcgi_LIBTOOLFLAGS = --tag CXX

wallet_service_cli_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir) -I$(srcdir)/secp256k1/include $(UNIVALUE_CFLAGS) $(LIBUNIVALUE_CFLAGS)
wallet_service_cli_CXXFLAGS = $(AM_CXXFLAGS)
wallet_service_cli_SOURCES = $(WALLET_SERVICE_COMMON_SOURCE) $(WALLET_SERVICE_CLI_SOURCE)
wallet_service_cli_LDADD = $(LIBUNIVALUE) $(EVENT_LIBS) $(EVENT_PTHREADS_LIBS) $(FCGI_LIBS) $(SQLITE_LIBS) $(LIBSECP256K1)
wallet_service_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)
wallet_service_cli_LIBTOOLFLAGS = --tag CXX



clean-local:
	-$(MAKE) -C univalue clean
	-$(MAKE) -C secp256k1 clean
	-rm -f config.h
