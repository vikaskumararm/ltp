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
	local cmd cmd2 cmd3 cmd4 cmd5

	cmd="tst_ipaddr_un 1 $1"
	cmd2="tst_ipaddr_un $1 $1"
	cmd3="tst_ipaddr_un -c $1"
	cmd4="tst_ipaddr_un -c $1 rhost"
	##tst_res TPASS "$1: '$cmd': $($cmd), '$cmd2': $($cmd2)"
	##tst_res TPASS "$1: '$cmd3': $($cmd3), '$cmd4': $($cmd4)"
	#tst_res TPASS "$1: '$cmd': $($cmd), '$cmd2': $($cmd2), '$cmd3': $($cmd3), '$cmd4': $($cmd4)"

	cmd5="tst_ipaddr_un -l 5 -m 253 $1 $1"
	cmd6="tst_ipaddr_un -l 5 -m 253 $1 $1 -c $1"
	cmd7="tst_ipaddr_un -l 5 -m 253 $1 $1 -c $1 rhost"
	##tst_res TPASS "$1: '$cmd5': $($cmd5)"
	##tst_res TPASS "$1: '$cmd6': $($cmd6), '$cmd7': $($cmd7)"
	#tst_res TPASS "$1: '$cmd5': $($cmd5) '$cmd6': $($cmd6), '$cmd7': $($cmd7)"

	cmd8="tst_ipaddr_un -c $1 -b"
	cmd9="tst_ipaddr_un -c $1 -b rhost"
	cmd10="tst_ipaddr_un -c $1 -n"
	cmd11="tst_ipaddr_un -c $1 -n rhost"
	##tst_res TINFO "$1: '$cmd8': $($cmd8), '$cmd9': $($cmd9)"
	##tst_res TINFO "$1: '$cmd10': $($cmd10), '$cmd11': $($cmd11)"
	#tst_res TPASS "$1: '$cmd8': $($cmd8), '$cmd9': $($cmd9), '$cmd10': $($cmd10), '$cmd11': $($cmd11)"

	cmd12="tst_ipaddr_un -c $1 -f"
	cmd13="tst_ipaddr_un -c $1 -f rhost"
	cmd14="tst_ipaddr_un -c $1 -l 3 -m 6"
	cmd15="tst_ipaddr_un -c $1 -l 3 -m 6 rhost"
	##tst_res TINFO "$1: '$cmd12': $($cmd12), '$cmd13': $($cmd13)"
	##tst_res TINFO "$1: '$cmd14': $($cmd14), '$cmd15': $($cmd15)"
	tst_res TPASS "$1: '$cmd12': $($cmd12), '$cmd13': $($cmd13), '$cmd14': $($cmd14), '$cmd15': $($cmd15)"

}

tst_run
