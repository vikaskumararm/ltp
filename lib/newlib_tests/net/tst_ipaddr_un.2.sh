#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_SETUP="setup"
TST_TESTFUNC="test_tst_ipaddr_un2"
TST_CNT=${NS_TIMES:-65537}
. tst_net.sh

setup()
{
	tst_res TINFO "testing tst_ipaddr_un $NS_TIMES times"

	test_tst_ipaddr_un2 -65537
	test_tst_ipaddr_un2 -2
	test_tst_ipaddr_un2 -1
	test_tst_ipaddr_un2 0

	tst_res TINFO "end of setup"
}

test_tst_ipaddr_un2()
{
	local cmd cmd2 cmd3 cmd4

	cmd="tst_ipaddr_un 1 $1"
	cmd2="tst_ipaddr_un $1 $1"
	cmd3="tst_ipaddr_un -c $1"
	cmd4="tst_ipaddr_un -c $1 rhost"
	tst_res TPASS "$1: '$cmd': $($cmd), '$cmd2': $($cmd2), '$cmd3': $($cmd3), '$cmd4': $($cmd4)"
}

tst_run
