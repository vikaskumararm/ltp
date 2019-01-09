#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC="do_test"
TST_OPTS="g"
TST_PARSE_ARGS="route_change_parse_args"
TST_SETUP="setup_gw"
TST_CLEANUP="cleanup_gw"
. tst_net.sh

route_change_parse_args()
{
	case "$1" in
	g) gw_flag="-g" ;;
	esac
}

setup_gw()
{
	#[ -n "$gw_flag" ] || return
	local cnt=1
	local max_net_id=255
	local gw_host=1

	[ "$TST_IPV6" ] && max_net_id=65535

	if [ -n "$gw_flag" ]; then
		tst_add_ipaddr -s -a $(tst_ipaddr_un $gw_host 1)
		tst_add_ipaddr -s -a $(tst_ipaddr_un $gw_host 2)
	fi

	while [ $cnt -le $NS_TIMES -a $cnt -lt $max_net_id ]; do
		if [ -n "$gw_flag" ]; then
			tst_add_ipaddr -s -a $(tst_ipaddr_un $gw_host $(($cnt + 2)))
		else
			echo tst_add_ipaddr -s -a $(tst_ipaddr_un $cnt 1) # FIXME: debug
			tst_add_ipaddr -s -a $(tst_ipaddr_un $cnt 1)
			tst_add_ipaddr -s -a $(tst_ipaddr_un $cnt 2)
			ping -c1 $(tst_ipaddr_un $cnt 1) # FIXME: debug
		fi
		cnt=$((cnt+1))
	done
}

cleanup_gw()
{
	#[ -n "$gw_flag" ] || return
	tst_restore_ipaddr
	tst_wait_ipv6_dad
}

do_test()
{
	local ip_flag

	[ "$TST_IPV6" ] && ip_flag="-6"
	EXPECT_PASS route-change -d $(tst_iface) -c $NS_TIMES $ip_flag $gw_flag
}

tst_run
