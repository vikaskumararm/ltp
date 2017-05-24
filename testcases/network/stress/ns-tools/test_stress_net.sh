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
# Author: Petr Vorel <pvorel@suse.cz>

# Library for all network/stress/ tests.

# using variables IPV4_NET16_UNUSED and various functions
. test_net.sh

tst_check_cmds ip killall

# NOTE: More information about these network variables can be found
# in testcases/network/stress/README

# TODO: Test expects prefix to be 24, which can get wrong when called
# with different prefix for IPV4_NET16_UNUSED.

# Netmask of for the tested network
IPV4_NETMASK="255.255.0.0"
IPV4_NETMASK_NUM=16

# Broadcast address of the tested network
IPV4_BROADCAST=${IPV4_NET16_UNUSED}.255.255
# Prefix of the Multicast Address
MCAST_IPV4_ADDR_PREFIX="224.10"
# Multicast Address
MCAST_IPV4_ADDR="${MCAST_IPV4_ADDR_PREFIX}.10.1"

LINK_NUM=${LINK_NUM:-0}
LHOST_IFNAME=$(tst_iface lhost $LINK_NUM)
RHOST_IFNAME=$(tst_iface rhost $LINK_NUM)

LINK_TOTAL=$(echo $LHOST_HWADDRS | wc -w)
RHOST_LINK_TOTAL=$(echo $RHOST_HWADDRS | wc -w)
[ $LINK_TOTAL -ne $RHOST_LINK_TOTAL ] && \
	tst_brkm TBROK "the number of element in LHOST_HWADDRS differs from RHOST_HWADDRS"

# Set an IPv4 address on local/remote interface.
# TYPE: { lhost | rhost }; Default value is 'lhost'.
# LINK_NUM: link number starting from 0; Default value is $LINK_NUM.
# IP: IP address to be set; Default value is $LHOST_IPV4_UNUSED (for lhost)
# or $RHOST_IPV4_UNUSED (for rhost).
tst_add_ipaddr_stress()
{
	local type="${1:-lhost}"
	local link_num="${2:-$LINK_NUM}"
	local ip="${3:-}"
	local mask=16
	local iface=$(tst_iface $type $link_num)

	if [ -z "$ip" ]; then
		[ $type = "lhost" ] && ip=$LHOST_IPV4_UNUSED || ip=$RHOST_IPV4_UNUSED
	fi

	cmd="ip a add $ip/$mask dev $iface"
	[ $type = "lhost" ] && ROD "$cmd" || tst_rhost_run -c "$cmd"

	return $?
}

restore_ipaddr()
{
	local type="${1:-lhost}"
	local link_num="${2:-$LINK_NUM}"
	local iface_loc=${3:-$(tst_iface lhost $link_num)}
	local iface_rmt=${4:-$(tst_iface rhost $link_num)}

	tst_restore_ipaddr $type $link_num || return $?
	tst_wait_ipv6_dad $iface_loc $iface_rmt
}

# Check connectivity with tst_ping.
# check_connectivity [IFACE] [DST ADDR]
# IFACE: source interface name
# DST ADDR: destination IPv4 or IPv6 address
# CNT: loop step (not required)
check_connectivity()
{
	local src_iface="${1:-$(tst_iface)}"
	local dst_addr="${2:-$(tst_ipaddr rhost)}"
	local cnt="${3:-}"
	local cnt_msg

	[ -n "$cnt" ] && cnt_msg=" (step $cnt)"

	tst_resm TINFO "check connectivity through $src_iface iface to ${dst_addr}$cnt_msg"

	tst_ping $src_iface $dst_addr
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "$src_iface is broken"
		return 1
	fi
	return 0
}

# Kill process on local/remote host.
# CMD: command to kill.
# TYPE: { lhost | rhost }; Default value is 'lhost'.
killall_sighup()
{
	local cmd="killall -q -SIGHUP $1"
	local type="${2:-lhost}"
	[ $type = "lhost" ] && $cmd || tst_rhost_run -c "$cmd"
}

# Kill process on remote host.
# CMD: command to kill.
killall_sighup_rhost()
{
	killall_sighup $1 rhost
}
