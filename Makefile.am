# Copyright (c) 2018 Jonas Schnelli
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

ACLOCAL_AMFLAGS = -I build-aux/m4
SUBDIRS = src
.PHONY: deploy FORCE

export PYTHONPATH

BITCOIN_WALLET_SERVICE_CGI=$(top_builddir)/src/wallet-service-cgi

empty :=
space := $(empty) $(empty)

$(BITCOIN_WALLET_SERVICE_CGI): FORCE
	$(MAKE) -C src $(@F)

dist_noinst_SCRIPTS = autogen.sh

