#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_NEEDS_ROOT=1
TST_SETUP="setup"
TST_CLEANUP="route_cleanup"
TST_NEEDS_CMDS="ip"
TST_CNT=260
TST_CNT=520
TST_CNT=20
TST_CNT=10000
TST_CNT=$((3*65535+50))

. tst_net.sh

route_cleanup()
{
	tst_res TINFO "lhost: '$lhost', rhost: '$rhost'" # FIXME: debug
	return # FIXME: debug
	tst_restore_ipaddr
	tst_restore_ipaddr rhost
}
