#!/bin/sh
# Copyright (c) International Business Machines  Corp., 2006
# Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Setup script for route4-* tests.
#
# More information about network parameters can be found
# in the following document: testcases/network/stress/README
#
# Author: Petr Vorel <pvorel@suse.cz>

. test_net_stress.sh

tst_check_cmds ip pgrep route

CHECK_INTERVAL=${CHECK_INTERVAL:-$(($NS_TIMES / 20))}

DST_NETWORK_OCTET_3=23
DST_HOST=5

route_cleanup()
{
	netstress_cleanup
	restore_ipaddr
	restore_ipaddr rhost
}

route_setup()
{
	netstress_setup
	tst_check_cmds route
	route_cleanup
}

manipulate_route()
{
	local cmd_name=$1
	local task=$2
	local network=$3
	local prefix=$4
	local netmask=$5
	local gateway=$6
	local ifname=$7

	[ "$task" = "add" ] || [ "$task" = "del" ] || tst_brkm TBROK "wrong task: '$task'"

	if [ "$cmd_name" = "ip" ]; then
		ROD "$cmd_name route $task $network/$prefix via $gateway dev $ifname"
	elif [ "$cmd_name" = "route" ]; then
		ROD "$cmd_name $task -net $network netmask $netmask gw $gateway dev $ifname"
	else
		tst_brkm TBROK "wrong command: '$task'"
	fi
}

get_cmd()
{
	local cmd_type=$1

	case $cmd_type in
	rt_cmd) echo 'route' ;;
	ip_cmd) echo 'ip' ;;
	ifconfig_cmd) echo 'ifconfig' ;;
	*) tst_brkm TBROK "Unknown test parameter '$cmd_type'"
	esac
}

# helper function for route4-change-{dst,gw,if} tests
route_test_change()
{
	local test_field="$1"
	local cmd_type="$2"
	local ip="$3"
	local dst_network="$(tst_ipaddr_un_ip $DST_NETWORK_OCTET_3)"
	local gateway="$(tst_ipaddr_un_host rhost)"
	local lhost_ifname="$(tst_iface)"
	local cmd_name="$(get_cmd $cmd_type)"
	local cnt=0 link_num=0
	local gateway2 lhost_ifname2 pre_dst_network test_field_name

	case $test_field in
	dst) test_field_name='destination' ;;
	gw) test_field_name='gateway' ;;
	if) test_field_name='interface' ;;
	*) tst_brkm TBROK "Unknown test parameter '$test_field'"
	esac

	tst_resm TINFO "the $test_field_name of an IPv4 route is changed by '$cmd_name' command $NS_TIMES times"

	if [ "$test_field" = 'dst' ]; then
		gateway="$ip"
		gateway2="$gateway"
	else
		dst_addr="$ip"
	fi

	while [ $cnt -lt $NS_TIMES ]; do
		lhost_ifname2="$lhost_ifname"
		gateway2="$gateway"

		pre_dst_network="$dst_network"
		if [ "$test_field" = 'dst' ]; then
			dst_addr="$(tst_ipaddr_un_ip $(($cnt % 255)) $DST_HOST)"
			dst_network="$(tst_ipaddr_un_ip $(($cnt % 255)))"
			netstress_cleanup
		elif [ "$test_field" = 'gw' ]; then
			gateway2="$(tst_ipaddr_un_host rhost $(($RHOST_COUNTER_START + $cnt)))"
			local cnt2=$(($cnt + 1))
			[ $cnt2 -eq $RHOST_COUNTER_COUNT ] && cnt2=$RHOST_COUNTER_START
			gateway="$(tst_ipaddr_un_host rhost $(($RHOST_COUNTER_START + $cnt2)))"
		elif [ "$test_field" = 'if' ]; then
			[ $link_num -ge $LINK_TOTAL ] && link_num=0
			lhost_ifname="$(tst_iface lhost $link_num)"
			gateway="$(tst_ipaddr_un_ip $link_num 1)"
			link_num=$(($link_num + 1))
		fi

		if [ $cnt -gt 0 ]; then
			manipulate_route $cmd_name 'del' $pre_dst_network $IPV4_NETMASK_NUM $IPV4_NETMASK $gateway2 $lhost_ifname2
		fi

		manipulate_route $cmd_name 'add' $dst_network $IPV4_NETMASK_NUM $IPV4_NETMASK $gateway $lhost_ifname
		make_background_tcp_traffic $(tst_ipaddr_un_host)
		check_connectivity_interval $cnt false $lhost_ifname $dst_addr || return

		cnt=$(($cnt + 1))
	done

	manipulate_route $cmd_name 'del' $dst_network $IPV4_NETMASK_NUM $IPV4_NETMASK $gateway $lhost_ifname

	tst_resm TPASS "test is finished correctly"
}
