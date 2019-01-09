#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC="do_test"
TST_OPTS="g"
TST_PARSE_ARGS="route_change_parse_args"
. tst_net.sh

route_change_parse_args()
{
	case "$1" in
	g) gw_flag="-g $(tst_ipaddr)" ;;
	esac
}

do_test()
{
	local ip_flag

	[ "$TST_IPV6" ] && ip_flag="-6"
	#echo "before"
	#tst_rhost_run -c "ip addr"
	#tst_rhost_run -c "ip addr add 10.23.0.1/24 dev $(tst_iface rhost)"
	#echo "after"
	#tst_rhost_run -c "ip addr"
	EXPECT_PASS route-change -d $(tst_iface) -c $NS_TIMES $ip_flag $gw_flag
}

tst_run
