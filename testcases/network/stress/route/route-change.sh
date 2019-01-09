#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC="do_test"
TST_OPTS="g"
TST_PARSE_ARGS="route_change_parse_args"
TST_SETUP="setup_gw"
TST_CLEANUP="setup_gw -d"
. tst_net.sh

route_change_parse_args()
{
	case "$1" in
	g) gw_flag="-g" ;;
	esac
}

setup_gw()
{
	[ -n "$gw_flag" ] || return

	local cmd="add"
	local cnt=0
	local mask=24
	local max_net_id=255

	if [ "$TST_IPV6" ]; then
		mask=64
		max_net_id=65535
	fi

    local OPTIND
	while getopts d opt; do
		case "$opt" in
		d) cmd="del" ;;
		*) tst_brk TBROK "add_gw: unknown option: $OPTARG" ;;
		esac
	done

	while [ $cnt -lt $NS_TIMES -a $cnt -lt $max_net_id ]; do
		ip addr $cmd $(tst_ipaddr_un $cnt 1)/$mask dev $(tst_iface)
		cnt=$((cnt+1))
	done
}

do_test()
{
	local ip_flag

	[ "$TST_IPV6" ] && ip_flag="-6"
	EXPECT_PASS route-change -d $(tst_iface) -c $NS_TIMES $ip_flag $gw_flag
}

tst_run
