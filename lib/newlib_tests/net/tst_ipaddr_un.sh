#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC=do_test
TST_CNT=2
TST_NEEDS_CMDS="cut"
. tst_net.sh

IPV4_DATA="
0 0|10.23.0.0
0 1|10.23.0.1
1 0|10.23.1.0
1 1|10.23.1.1
1 2|10.23.1.2
2 2|10.23.2.2
1 3|10.23.1.3
3 3|10.23.3.3
1 128|10.23.1.128
128 128|10.23.128.128
1 254|10.23.1.254
254 254|10.23.254.254
1 255|10.23.1.255
255 255|10.23.255.255
1 256|10.23.1.0
256 256|10.23.0.0
1 257|10.23.1.1
257 257|10.23.1.1

-c 0|10.23.0.2
-c 0 rhost|10.23.0.1
-c 1|10.23.0.2
-c 1 rhost|10.23.0.1
-c 2|10.23.0.4
-c 2 rhost|10.23.0.3
-c 127|10.23.0.254
-c 127 rhost|10.23.0.253
-c 128|10.23.1.2
-c 128 rhost|10.23.1.1
-c 254|10.23.1.254
-c 254 rhost|10.23.1.253
-c 255|10.23.2.2
-c 255 rhost|10.23.2.1

-c 0 -b|10.23.0.2
-c 0 -b rhost|10.23.0.1
-c 1 -b|10.23.0.2
-c 1 -b rhost|10.23.0.1
-c 127 -b|10.23.0.254
-c 127 -b rhost|10.23.0.253
-c 128 -b|10.23.1.1
-c 128 -b rhost|10.23.0.255
-c 255 -b|10.23.1.255
-c 255 -b rhost|10.23.1.254
-c 256 -b|10.23.2.2
-c 256 -b rhost|10.23.2.1

-c 0 -f|10.23.0.1
-c 0 -f rhost|10.23.0.0
-c 1 -f|10.23.0.1
-c 1 -f rhost|10.23.0.0
-c 127 -f|10.23.0.253
-c 127 -f rhost|10.23.0.252
-c 128 -f|10.23.1.0
-c 128 -f rhost|10.23.0.254
-c 255 -f|10.23.1.254
-c 255 -f rhost|10.23.1.253
-c 256 -f|10.23.2.1
-c 256 -f rhost|10.23.2.0

-c 0 -n|10.23.0.1
-c 0 -n rhost|10.23.0.0
-c 1 -n|10.23.0.1
-c 1 -n rhost|10.23.0.0
-c 127 -n|10.23.0.253
-c 127 -n rhost|10.23.0.252
-c 128 -n|10.23.1.1
-c 128 -n rhost|10.23.1.0
-c 255 -n|10.23.2.1
-c 255 -n rhost|10.23.2.0
-c 256 -n|10.23.2.3
-c 256 -n rhost|10.23.2.2

-l 5 -m 253 0 0|10.23.0.5
-l 5 -m 253 0 0 -c 0|10.23.0.5
-l 5 -m 253 0 0 -c 0 rhost|10.23.0.5
-l 5 -m 253 1 1|10.23.1.6
-l 5 -m 253 1 1 -c 1|10.23.1.6
-l 5 -m 253 1 1 -c 1 rhost|10.23.1.6
-l 5 -m 253 248 248|10.23.248.253
-l 5 -m 253 248 248 -c 248|10.23.248.253
-l 5 -m 253 248 248 -c 248 rhost|10.23.248.253
-l 5 -m 253 249 249|10.23.249.5
-l 5 -m 253 249 249 -c 249|10.23.249.5
-l 5 -m 253 249 249 -c 249 rhost|10.23.249.5

-c 0 -l 3 -m 6|10.23.0.4
-c 0 -l 3 -m 6 rhost|10.23.0.3
-c 1 -l 3 -m 6|10.23.0.4
-c 1 -l 3 -m 6 rhost|10.23.0.3
-c 2 -l 3 -m 6|10.23.0.6
-c 2 -l 3 -m 6 rhost|10.23.0.5
-c 3 -l 3 -m 6|10.23.0.4
-c 3 -l 3 -m 6 rhost|10.23.0.3
-c 255 -l 3 -m 6|10.23.84.4
-c 255 -l 3 -m 6 rhost|10.23.84.3
"

IPV6_DATA="
0 0|fd00:23::
0 1|fd00:23::1
1 0|fd00:23:1::
1 1|fd00:23:1::1
1 2|fd00:23:1::2
2 2|fd00:23:2::2
1 3|fd00:23:1::3
3 3|fd00:23:3::3
1 32767|fd00:23:1::7fff
32767 32767|fd00:23:7fff::7fff
1 65534|fd00:23:1::fffe
65534 65534|fd00:23:fffe::fffe
1 65535|fd00:23:1::ffff
65535 65535|fd00:23:ffff::ffff
1 65536|fd00:23:1::
65536 65536|fd00:23::
1 65537|fd00:23:1::1
65537 65537|fd00:23:1::1

-c 0|fd00:23::2
-c 0 rhost|fd00:23::1
-c 1|fd00:23::2
-c 1 rhost|fd00:23::1
-c 2|fd00:23::4
-c 2 rhost|fd00:23::3
-c 32767|fd00:23::fffe
-c 32767 rhost|fd00:23::fffd
-c 32768|fd00:23:1::2
-c 32768 rhost|fd00:23:1::1
-c 65534|fd00:23:1::fffe
-c 65534 rhost|fd00:23:1::fffd
-c 65535|fd00:23:2::2
-c 65535 rhost|fd00:23:2::1

-c 0 -b|fd00:23::2
-c 0 -b rhost|fd00:23::1
-c 1 -b|fd00:23::2
-c 1 -b rhost|fd00:23::1
-c 32767 -b|fd00:23::fffe
-c 32767 -b rhost|fd00:23::fffd
-c 32768 -b|fd00:23:1::1
-c 32768 -b rhost|fd00:23::ffff
-c 65535 -b|fd00:23:1::ffff
-c 65535 -b rhost|fd00:23:1::fffe
-c 65536 -b|fd00:23:2::2
-c 65536 -b rhost|fd00:23:2::1

-c 0 -f|fd00:23::1
-c 0 -f rhost|fd00:23::
-c 1 -f|fd00:23::1
-c 1 -f rhost|fd00:23::
-c 32767 -f|fd00:23::fffd
-c 32767 -f rhost|fd00:23::fffc
-c 32768 -f|fd00:23:1::
-c 32768 -f rhost|fd00:23::fffe
-c 65535 -f|fd00:23:1::fffe
-c 65535 -f rhost|fd00:23:1::fffd
-c 65536 -f|fd00:23:2::1
-c 65536 -f rhost|fd00:23:2::

-c 0 -n|fd00:23::1
-c 0 -n rhost|fd00:23::
-c 1 -n|fd00:23::1
-c 1 -n rhost|fd00:23::
-c 32767 -n|fd00:23::fffd
-c 32767 -n rhost|fd00:23::fffc
-c 32768 -n|fd00:23:1::1
-c 32768 -n rhost|fd00:23:1::
-c 65535 -n|fd00:23:2::1
-c 65535 -n rhost|fd00:23:2::
-c 65536 -n|fd00:23:2::3
-c 65536 -n rhost|fd00:23:2::2

-l 5 -m 253 0 0|fd00:23::5
-l 5 -m 253 0 0 -c 0|fd00:23::5
-l 5 -m 253 0 0 -c 0 rhost|fd00:23::5
-l 5 -m 253 1 1|fd00:23:1::6
-l 5 -m 253 1 1 -c 1|fd00:23:1::6
-l 5 -m 253 1 1 -c 1 rhost|fd00:23:1::6
-l 5 -m 253 0 0|fd00:23::5
-l 5 -m 253 0 0 -c 0|fd00:23::5
-l 5 -m 253 0 0 -c 0 rhost|fd00:23::5
-l 5 -m 253 1 1|fd00:23:1::6
-l 5 -m 253 249 249 -c 249|fd00:23:f9::5
-l 5 -m 253 249 249 -c 249 rhost|fd00:23:f9::5

-c 0 -l 3 -m 6|fd00:23::4
-c 0 -l 3 -m 6 rhost|fd00:23::3
-c 1 -l 3 -m 6|fd00:23::4
-c 1 -l 3 -m 6 rhost|fd00:23::3
-c 2 -l 3 -m 6|fd00:23::6
-c 2 -l 3 -m 6 rhost|fd00:23::5
-c 3 -l 3 -m 6|fd00:23::4
-c 3 -l 3 -m 6 rhost|fd00:23::3
-c 65535 -l 3 -m 6|fd00:23:5554::4
-c 65535 -l 3 -m 6 rhost|fd00:23:5554::3
"

test_tst_ipaddr_un()
{
	local data cmd i result
	local var="$1"

	tst_res TINFO "Testing for IPv${TST_IPVER}, data: \$$var"

	eval data="\$$var"
	IFS="
"
	for i in $data; do
		cmd="tst_ipaddr_un $(echo $i | cut -d'|' -f 1)"
		result="$(echo $i | cut -d'|' -f 2)"
		tst_res TINFO "testing $cmd"
		EXPECT_PASS "[ '$(eval $cmd)' = '$result' ]"
	done
}

do_test2()
{
	test_tst_ipaddr_un "IPV${TST_IPVER}_DATA"
}

do_test()
{
	case $1 in
	 1) TST_IPV6= TST_IPVER=4 do_test2;;
	 2) TST_IPV6=6 TST_IPVER=6 do_test2;;
	esac
}

tst_run
