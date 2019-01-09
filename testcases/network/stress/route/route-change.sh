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
	local dir i rhost
	tst_restore_ipaddr
	tst_restore_ipaddr rhost

	[ "$TST_IPV6" ] && max_net_id=65535

	while [ $cnt -le $NS_TIMES -a $cnt -lt $max_net_id ]; do
		rhost="$(tst_ipaddr_un $cnt 3)"
		server="$rhost $server"
		tst_add_ipaddr -s -a $rhost rhost
		tst_add_ipaddr -s -a $(tst_ipaddr_un $cnt 2)
		cnt=$((cnt+1))
	done

	[ "$TST_IPV6" ] && ip_flag="-6"
	echo "route-change-server $ip_flag -B $TST_TMPDIR -c $NS_TIMES -D $(tst_get_ifaces rhost) -S '$server' &" # FIXME: debug
	tst_rhost_run -s -c "route-change-server $ip_flag -B $TST_TMPDIR -c $NS_TIMES -D $(tst_get_ifaces rhost) -S '$server'" > log.txt &

	for i in $(seq 1 50); do
		tst_rhost_run -c "[ -f $TST_TMPDIR/ready ]"
		[ $? -eq 0 ] && break
		[ $(($i % 10)) -eq 0 ] && \
			tst_res_ TINFO "wait for route-change-server $((i / 10))/5 sec"
		tst_sleep 100ms
	done
	tst_rhost_run -s -c "ls -la $TST_TMPDIR/ready; cat $TST_TMPDIR/ready" # FIXME: debug

	cnt=1
	while [ $cnt -le $NS_TIMES -a $cnt -lt $max_net_id ]; do
		dir="$TST_TMPDIR/$cnt/"
		echo "server config $cnt:" # FIXME: debug
		tst_rhost_run -s -c "cat $dir/ip; printf ':'; cat $dir/port"
		cnt=$((cnt+1))
	done
	#tst_brk TBROK "DEVEL" # FIXME: debug
	#exit 55 # FIXME: debug
}

cleanup()
{
	tst_res TINFO "=== in cleanup ===" # FIXME: debug
	# FIXME: debug
	echo "* find $TST_TMPDIR"
	find $TST_TMPDIR
	echo "* cat"
	for i in $(find $TST_TMPDIR -type f); do echo "cat $i"; cat $i; echo "----"; done

	echo "kill" # FIXME: debug
	pkill 'route-change$'
	pkill 'route-change-server$'

	for i in $(ps aux|grep [r]oute-change | awk '{print $2}'); do echo "killing $i"; kill $i; done

	tst_restore_ipaddr
	tst_restore_ipaddr rhost
}

do_test()
{
	EXPECT_PASS route-change -d $(tst_iface) -B $TST_TMPDIR -c $NS_TIMES $ip_flag $gw_flag

	echo "==== route-change-server log ===" # FIXME: debug
	cat log.txt
	echo "====="
}

tst_run
