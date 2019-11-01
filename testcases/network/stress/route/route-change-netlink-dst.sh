#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines Corp., 2006
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
# Rewrite into new shell API: Petr Vorel
#
# Change route destination
# lhost: 10.0.0.2, rhost: 10.23.x.1

TST_TESTFUNC="test_dst"
#. route-lib.sh
. tst_net.sh # FIXME: debug

setup()
{
	tst_res TINFO "change IPv$TST_IPVER route destination $NS_TIMES times"

	local iface="$(tst_iface)"
	local rhost rt

	local cnt=0
	local max_ip_limit=254
	[ "$TST_IPV6" ] && max_ip_limit=65534
	local gw

	tst_is_int $MAX_IP || tst_brk TBROK "\$MAX_IP not int ($MAX_IP)"
	[ "$MAX_IP" -gt $max_ip_limit ] && MAX_IP=$max_ip_limit

	while [ $cnt -lt $NS_TIMES -a $cnt -lt $MAX_IP ]; do
		rt="$(tst_ipaddr_un -p $cnt)"
		rhost="$(tst_ipaddr_un $cnt 1)"

		tst_add_ipaddr -s -q -a $rhost rhost
		ROD ip route add $rt dev $iface
		#EXPECT_PASS_BRK ping$TST_IPV6 -c1 -I $(tst_ipaddr) $rhost \>/dev/null
		#ROD ip route del $rt dev $iface
		#tst_del_ipaddr -s -q -a $rhost rhost

		cnt=$((cnt+1))
	done
	MAX_IP=$cnt
}

test_dst()
{
	local ip_flag
	[ "$TST_IPV6" ] && ip_flag="-6"

	# TODO: adapt from concept the same ip on lhost and rhost to:
	# lhost: 10.0.0.2, rhost: 10.23.x.1
	#EXPECT_PASS route-change-netlink -c $NS_TIMES -d $(tst_iface) $ip_flag $g_opt -l $LHOST -m $MAX_IP -r $RHOST
}

tst_run
