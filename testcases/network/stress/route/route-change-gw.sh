#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

# Change route gateway
# lhost: 10.23.x.2, gw (on rhost): 10.23.x.1, rhost: 10.23.0.1

TST_TESTFUNC="test_gw"
. route-lib.sh

setup()
{
	tst_res TINFO "change IPv$TST_IPVER route gateway $NS_TIMES times"

	rt="$(tst_ipaddr_un -m 0 0)"
	lhost="$(tst_ipaddr_un 1 1)"
	rhost="$(tst_ipaddr_un 0 1)"
	tst_res TINFO "lhost: '$lhost', rhost: '$rhost'" # FIXME: debug
	return # FIXME: debug
	tst_add_ipaddr -s -a $lhost
	tst_add_ipaddr -s -a $rhost rhost
}

test_gw()
{
	#local gw="$(tst_ipaddr_un 1 $(($1 + 1)))"
	#local gw2="$(tst_ipaddr_un 1 $(($1 % 254 + 1)))"
	#local gw3="$(tst_ipaddr_un -c $(($1 + 1)))"
	#local gw3="$(tst_ipaddr_un 1 $(($1 % 253 + 2)))"
	#local gw4="$(tst_ipaddr_un -l 2 1 $(($1)))"
	#local gw5="$(tst_ipaddr_un -l 2 1 $(($1 + 1)))"
	#echo "(tst_ipaddr_un -l 5 1 $(($1)))"
	#local gw6="$(tst_ipaddr_un -l 5 1 $(($1)))"
	#local iface="$(tst_iface)"
	#tst_res TPASS "$1, gw3: '$gw3', gw4: '$gw4', gw5: '$gw5', gw6: '$gw6'" # FIXME: debug
	#tst_brk TBROK "DEVEL" # FIXME: debug
	local gw="$(tst_ipaddr_un 1 $1)"
	#local gw2="$(tst_ipaddr_un -l2 1 $1)"
	#local gw5="$(tst_ipaddr_un -l5 1 $1)"
	local gw2="$(tst_ipaddr_un -l2 1 $(($1-1)))"
	local gw5="$(tst_ipaddr_un -l5 1 $(($1-1)))"
	tst_res TPASS "$1, gw: '$gw', gw2: '$gw2', gw5: '$gw5'" # FIXME: debug
	return # FIXME: debug

	tst_res TINFO "testing route over gateway '$gw'"

	tst_add_ipaddr -s -a $gw rhost
	ROD ip route replace $rt dev $iface via $gw
	EXPECT_PASS ping$TST_IPV6 -c1 -I $lhost $rhost
	ROD ip route del $rt dev $iface via $gw
	tst_del_ipaddr -s -a $gw rhost
}

tst_run
