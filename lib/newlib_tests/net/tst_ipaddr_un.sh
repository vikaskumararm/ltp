#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC="test_tst_ipaddr_un"
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="ip"
. tst_net.sh

test_tst_ipaddr_un()
{
	local ip ipc ipcr
	local m=$((255*255))

	for i in \
		{-1..20} \
		{250..260} \
		{509..512} \
		$((m-2)) $((m-1)) $m \
	; do
		ip="$(tst_ipaddr_un 1 $i)"
		ip5="$(tst_ipaddr_un -m5 -p 1 $i)"
		ipc="$(tst_ipaddr_un -c $i)"
		ipc6="$(tst_ipaddr_un -m6 -c $i)"
		ipcr="$(tst_ipaddr_un -c $i rhost)"
		ipcr10="$(tst_ipaddr_un -m10 -c $i rhost)"
		tst_res TPASS "ip: '$ip', ip5: '$ip5'; ipc: '$ipc', ipc6: '$ipc6', ipcr: '$ipcr', ipcr10: '$ipcr10' ($i)"
	done
}

tst_run
