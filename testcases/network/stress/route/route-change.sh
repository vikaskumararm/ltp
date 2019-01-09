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
	local cnt=1
	local mask=24
	local max_net_id=255
	local dad

	if [ "$TST_IPV6" ]; then
		mask=64
		max_net_id=65535
		dad="nodad"
	fi

    local OPTIND
	while getopts d opt; do
		case "$opt" in
		d) cmd="del" ;;
		*) tst_brk TBROK "add_gw: unknown option: $OPTARG" ;;
		esac
	done

	while [ $cnt -le $NS_TIMES -a $cnt -lt $max_net_id ]; do
		[ "$cmd" = "add" ] && { j="BEFORE"; echo "$j local"; ip -$TST_IPVER a; ip -$TST_IPVER r; echo "$j remote"; tst_rhost_run -s -c "ip -$TST_IPVER addr; ip -$TST_IPVER r"; echo "====="; } # FIXME: debug

		# FIXME: debug WHY is needed different address for C and shell? (1 instead of 2)
		ROD ip addr $cmd $(tst_ipaddr_un $cnt 2)/$mask dev $(tst_iface) $dad
		echo "tst_rhost_run -s -c 'ip addr $cmd $(tst_ipaddr_un $cnt 3)/$mask dev $(tst_iface rhost) $dad'" # FIXME: debug
		tst_rhost_run -s -c "ip addr $cmd $(tst_ipaddr_un $cnt 3)/$mask dev $(tst_iface rhost) $dad"

		# FIXME: debug
		#echo ping -c 1 $(tst_ipaddr_un $cnt 55) -I $(tst_iface)
		#ping -c 1 $(tst_ipaddr_un $cnt 55) -I $(tst_iface)
		#echo "tst_rhost_run -s -c 'ping -c 1 $(tst_ipaddr_un $cnt 55) -I $(tst_iface rhost)'"
		#tst_rhost_run -c "ping -c 1 $(tst_ipaddr_un $cnt 55) -I $(tst_iface rhost)"

		[ "$cmd" = "add" ] && { j="AFTER"; echo "$j local"; ip -$TST_IPVER a; ip -$TST_IPVER r; echo "$j remote"; tst_rhost_run -s -c "ip -$TST_IPVER addr; ip -$TST_IPVER r"; echo "====="; } # FIXME: debug
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
