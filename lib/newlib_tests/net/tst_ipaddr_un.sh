#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC=do_test
TST_CNT=2
. tst_net.sh

max_net_id_ipv4=255
max_net_id_ipv6=65535
m4=$((max_net_id_ipv4*max_net_id_ipv4))
m6=$((max_net_id_ipv6*max_net_id_ipv6))

# params: none
IPV4_DATA="
-1^10.23.1.1|10.23.0.2|10.23.0.1
0^10.23.1.0|10.23.0.2|10.23.0.1
1^10.23.1.1|10.23.0.2|10.23.0.1
2^10.23.1.2|10.23.0.4|10.23.0.3
3^10.23.1.3|10.23.0.6|10.23.0.5
253^10.23.1.253|10.23.1.252|10.23.1.251
254^10.23.1.254|10.23.1.254|10.23.1.253
255^10.23.1.0|10.23.2.2|10.23.2.1
256^10.23.1.1|10.23.2.4|10.23.2.3
$((m4-2))^10.23.1.253|10.23.1.252|10.23.1.251
$((m4-1))^10.23.1.254|10.23.1.254|10.23.1.253
$m4^10.23.1.0|10.23.2.2|10.23.2.1
$((m4+1))^10.23.1.1|10.23.2.4|10.23.2.3
$((m4+2))^10.23.1.2|10.23.2.6|10.23.2.5
"
IPV6_DATA="
-1^fd00:23:1::1|fd00:23::2|fd00:23::1
0^fd00:23:1::|fd00:23::2|fd00:23::1
1^fd00:23:1::1|fd00:23::2|fd00:23::1
2^fd00:23:1::2|fd00:23::4|fd00:23::3
3^fd00:23:1::3|fd00:23::6|fd00:23::5
253^fd00:23:1::fd|fd00:23::1fa|fd00:23::1f9
254^fd00:23:1::fe|fd00:23::1fc|fd00:23::1fb
255^fd00:23:1::ff|fd00:23::1fe|fd00:23::1fd
256^fd00:23:1::100|fd00:23::200|fd00:23::1ff
$((m6-2))^fd00:23:1::fffd|fd00:23:1::fffc|fd00:23:1::fffb
$((m6-1))^fd00:23:1::fffe|fd00:23:1::fffe|fd00:23:1::fffd
$m6^fd00:23:1::|fd00:23:2::2|fd00:23:2::1
$((m6+1))^fd00:23:1::1|fd00:23:2::4|fd00:23:2::3
$((m6+2))^fd00:23:1::2|fd00:23:2::6|fd00:23:2::5
"

# params: -n
IPV4_DATA_N="
-1^10.23.1.1|10.23.0.2|10.23.0.1
0^10.23.1.0|10.23.0.2|10.23.0.1
1^10.23.1.1|10.23.0.2|10.23.0.1
2^10.23.1.2|10.23.0.4|10.23.0.3
3^10.23.1.3|10.23.0.6|10.23.0.5
253^10.23.1.253|10.23.1.252|10.23.1.251
254^10.23.1.254|10.23.1.254|10.23.1.253
255^10.23.1.0|10.23.2.2|10.23.2.1
256^10.23.1.1|10.23.2.4|10.23.2.3
$((m4-2))^10.23.1.253|10.23.1.252|10.23.1.251
$((m4-1))^10.23.1.254|10.23.1.254|10.23.1.253
$m4^10.23.1.0|10.23.2.2|10.23.2.1
$((m4+1))^10.23.1.1|10.23.2.4|10.23.2.3
$((m4+2))^10.23.1.2|10.23.2.6|10.23.2.5
"
IPV6_DATA_N="
-1^fd00:23:1::1|fd00:23::2|fd00:23::1
0^fd00:23:1::|fd00:23::2|fd00:23::1
1^fd00:23:1::1|fd00:23::2|fd00:23::1
2^fd00:23:1::2|fd00:23::4|fd00:23::3
3^fd00:23:1::3|fd00:23::6|fd00:23::5
253^fd00:23:1::fd|fd00:23::1fa|fd00:23::1f9
254^fd00:23:1::fe|fd00:23::1fc|fd00:23::1fb
255^fd00:23:1::ff|fd00:23::1fe|fd00:23::1fd
256^fd00:23:1::100|fd00:23::200|fd00:23::1ff
$((m6-2))^fd00:23:1::fffd|fd00:23:1::fffc|fd00:23:1::fffb
$((m6-1))^fd00:23:1::fffe|fd00:23:1::fffe|fd00:23:1::fffd
$m6^fd00:23:1::|fd00:23:2::2|fd00:23:2::1
$((m6+1))^fd00:23:1::1|fd00:23:2::4|fd00:23:2::3
$((m6+2))^fd00:23:1::2|fd00:23:2::6|fd00:23:2::5
"

test_tst_ipaddr_un()
{
	local data i j results
	local var="$1"
	local params="$2"

	tst_res TINFO "Testing for IPv${TST_IPVER}, data: \$$var"

	eval data="\$$var"
	for i in $data; do
		j="$(echo $i | cut -d^ -f 1)"
		results="$(echo $i | cut -d^ -f 2)"

		tst_res TINFO "counter / host_id: $j, params: '$params'"
		EXPECT_PASS "[ $(tst_ipaddr_un $params 1 $j) = $(echo $results | cut -d'|' -f 1) ]"
		EXPECT_PASS "[ $(tst_ipaddr_un $params -c $j) = $(echo $results | cut -d'|' -f 2) ]"
		EXPECT_PASS "[ $(tst_ipaddr_un $params -c $j rhost) = $(echo $results | cut -d'|' -f 3) ]"
	done
}

do_test2()
{
	test_tst_ipaddr_un "IPV${TST_IPVER}_DATA" # previous default (now incompatible) # FIXME: debug
	#test_tst_ipaddr_un "IPV"$TST_IPVER"_DATA_N" "-n" # compatible previous default # OK
}

do_test()
{
	case $1 in
	 1) TST_IPV6= TST_IPVER=4 do_test2;;
	 2) TST_IPV6=6 TST_IPVER=6 do_test2;;
	esac
}

tst_run
