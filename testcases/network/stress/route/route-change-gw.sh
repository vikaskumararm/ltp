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

	[ "$TST_IPV6" ] && next_net=32768 || next_net=128

	rt="$(tst_ipaddr_un -p 0 0)"
	lhost="$(tst_ipaddr_un -c $next_net rhost)"
	rhost="$(tst_ipaddr_un 0 1)"
	tst_add_ipaddr -s -a $lhost
	tst_add_ipaddr -s -a $rhost rhost
}

test_gw()
{
	local gw="$(tst_ipaddr_un -c $(($next_net+$1)))"
	local iface="$(tst_iface)"

	echo "$i: $1" # FIXME: debug
	tst_res TINFO "testing route over gateway '$gw'"

	# FIXME: debug
	if [ $1 = 254 ]; then
		tst_res TPASS "need to skip gw: '$gw'"
		return
	fi

	tst_add_ipaddr -s -a $gw rhost
	ROD ip route replace $rt dev $iface via $gw
	EXPECT_PASS ping$TST_IPV6 -c1 -I $lhost $rhost
	ROD ip route del $rt dev $iface via $gw
	tst_del_ipaddr -s -a $gw rhost
}

tst_run
