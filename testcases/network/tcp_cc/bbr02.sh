#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.

TST_SETUP="setup"
TST_TESTFUNC="do_test"
TST_CLEANUP="cleanup"
TST_MIN_KVER="4.13"

. tcp_cc_lib.sh

TST_CLEANUP="cleanup"

cleanup()
{
	tc qdisc del dev $(tst_iface) root netem

	tcp_cc_cleanup
}

setup()
{
	tcp_cc_setup

	local emu_opts="delay 5ms 1ms 20% loss 0.3% ecn corrupt \
0.1% reorder 93% 50% limit 10000"

	tst_res TINFO "emulate congestion with packet $emu_opts"
	ROD tc qdisc add dev $(tst_iface) root netem $emu_opts
}

do_test()
{
	for q in $qdisc_list; do
		if tc qdisc add $q >/dev/null 2>&1; then
			tst_res TCONF "$q qdisc not supported"
			continue
		fi

		tcp_cc_set_qdisc $q
		tcp_cc_test01 bbr -50
	done
}

tst_run
