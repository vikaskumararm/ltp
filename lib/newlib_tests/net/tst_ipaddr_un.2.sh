#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_SETUP="setup"
TST_TESTFUNC="test_tst_ipaddr_un"

TST_CNT=${NS_TIMES:-65537} # IPv6
[ -z "$NS_TIMES" -a $# -eq 0 ] && TST_CNT=257 # IPv4

. tst_net.sh

setup()
{
	tst_res TINFO "testing tst_ipaddr_un $NS_TIMES times"

	# Illegal option -6
	#test_tst_ipaddr_un -65537
	#test_tst_ipaddr_un -2
	#test_tst_ipaddr_un -1
	test_tst_ipaddr_un 0

	tst_res TINFO "end of setup"
}

test_tst_ipaddr_un()
{
	local cmd cmd2 cmd3 cmd4

	cmd="tst_ipaddr_un 1 $1"
	cmd2="tst_ipaddr_un $1 $1"
	cmd3="tst_ipaddr_un -c $1"
	cmd4="tst_ipaddr_un -c $1 rhost"
	tst_res TPASS "$1: '$cmd': $($cmd), '$cmd2': $($cmd2), '$cmd3': $($cmd3), '$cmd4': $($cmd4)"
}

tst_run
