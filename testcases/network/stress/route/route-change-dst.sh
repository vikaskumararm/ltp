#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC="do_test"
. tst_net.sh

do_test()
{
	local ip_flag

	[ "$TST_IPV6" ] && ip_flag="-6"
	EXPECT_PASS route-change-dst -d $(tst_iface) -c $NS_TIMES $ip_flag
}

tst_run
