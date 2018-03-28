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

	rt="$(tst_ipaddr_un -p 0 0)"
	lhost="$(tst_ipaddr_un 1 1)"
	rhost="$(tst_ipaddr_un 0 1)"
	tst_add_ipaddr -s -a $lhost
	tst_add_ipaddr -s -a $rhost rhost
}

test_gw()
{
	#local gw_old="$(tst_ipaddr_un 1 $(($1 + 1)))"
	local gw="$(tst_ipaddr_un 1 $(($1 + 1)))"
	#local gw="$(tst_ipaddr_un 1 $(($1 % 253 + 2)))"

	local iface="$(tst_iface)"

	echo "i: $1 (gw: $gw, gw_old: $gw_old)" # FIXME: debug
	tst_res TINFO "testing route over gateway '$gw'"

	tst_add_ipaddr -s -a $gw rhost
	ROD ip route replace $rt dev $iface via $gw
	EXPECT_PASS ping$TST_IPV6 -c1 -I $lhost $rhost
	ROD ip route del $rt dev $iface via $gw
	tst_del_ipaddr -s -a $gw rhost
}

tst_run
