# Copyright (c) 2018 Jonas Schnelli
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

DIST_SUBDIRS = univalue

AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS)
AM_CXXFLAGS = $(HARDENED_CXXFLAGS) $(ERROR_CXXFLAGS)
AM_CPPFLAGS = $(HARDENED_CPPFLAGS)
EXTRA_LIBRARIES =

LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

bin_PROGRAMS = wallet-service-cgi

WALLET_SERVICE_CGI_H = \
    compat.h \
    hash.h \
    logging.h \
    netaddress.h \
    shutdown.h \
    sync.h \
    threadsafety.h \
    tinyformat.h \
    utils.h
    
WALLET_SERVICE_CGI_SOURCE = \
    logging.cpp \
    main.cpp \
    netaddress.cpp \
    shutdown.cpp \
    sync.cpp \
    utils.cpp \
    $(BITCOINCORE_INDEXD_H)

# application binary #
wallet_service_cgi_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir) $(UNIVALUE_CFLAGS) $(LIBUNIVALUE_CFLAGS)
wallet_service_cgi_CXXFLAGS = $(AM_CXXFLAGS)
wallet_service_cgi_SOURCES = $(WALLET_SERVICE_CGI_SOURCE)
wallet_service_cgi_LDADD = $(LIBUNIVALUE) $(EVENT_LIBS) $(EVENT_PTHREADS_LIBS) $(FCGI_LIBS)
wallet_service_cgi_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)
wallet_service_cgi_LIBTOOLFLAGS = --tag CXX


clean-local:
	-$(MAKE) -C univalue clean
	-rm -f config.h
