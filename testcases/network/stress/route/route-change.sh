#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC="do_test"
TST_OPTS="g"
TST_PARSE_ARGS="route_change_parse_args"
TST_SETUP="setup"
TST_CLEANUP="cleanup"
TST_NEEDS_TMPDIR=1
. tst_net.sh

route_change_parse_args()
{
	case "$1" in
	g) gw_flag="-g" ;;
	esac
}

setup()
{
	local cnt=1
	local max_net_id=255

	[ "$TST_IPV6" ] && max_net_id=65535

	while [ $cnt -le $NS_TIMES -a $cnt -lt $max_net_id ]; do
		tst_add_ipaddr -s -a $(tst_ipaddr_un $cnt 3) rhost
		tst_add_ipaddr -s -a $(tst_ipaddr_un $cnt 2)
		cnt=$((cnt+1))
	done
}

cleanup()
{
	tst_restore_ipaddr
	tst_wait_ipv6_dad
}

do_test()
{
	local ip_flag port
	[ "$TST_IPV6" ] && ip_flag="-6"

	EXPECT_PASS route-change -d $(tst_iface) -c $NS_TIMES $ip_flag $gw_flag
}

tst_run
