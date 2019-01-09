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
	g) g_opt="-g" ;;
	esac
}

setup()
{
	local cnt=1
	local max_net_id=255
	local lhost rhost

	[ "$TST_IPV6" ] && max_net_id=65535

	d_opt="$(tst_ipaddr_un 0 0)"
	while [ $cnt -le $NS_TIMES -a $cnt -lt $max_net_id ]; do
		lhost="$(tst_ipaddr_un $cnt 2)"
		rhost="$(tst_ipaddr_un $cnt 3)"

		if [ "$g_opt" ]; then
			g_opt="$g_opt $(tst_ipaddr_un $cnt 1)"
		else
			d_opt="$(tst_ipaddr_un $cnt 0) $d_opt"
		fi

		l_opt="$lhost $l_opt"
		r_opt="$rhost $r_opt"

		tst_add_ipaddr -s -a $lhost
		tst_add_ipaddr -s -a $rhost rhost
		cnt=$((cnt+1))
	done
	d_opt="-d $d_opt"
	l_opt="-l $l_opt"
	r_opt="-r $r_opt"
}

cleanup()
{
	tst_restore_ipaddr
	tst_wait_ipv6_dad
}

do_test()
{
	local ip_opt port
	[ "$TST_IPV6" ] && ip_opt="-6"

	echo EXPECT_PASS route-change -c $NS_TIMES -D $(tst_iface) $ip_opt "$d_opt" "$g_opt" "$l_opt" "$r_opt" # FIXME: debug
	EXPECT_PASS route-change -c $NS_TIMES -D $(tst_iface) $ip_opt "$d_opt" "$g_opt" "$l_opt" "$r_opt"
}

tst_run
