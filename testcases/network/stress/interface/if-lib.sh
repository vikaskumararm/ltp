#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
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
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TST_CLEANUP="cleanup"

. test_stress_net.sh

ipver=${TST_IPV6:-4}

setup()
{
	tst_require_root
	tst_check_cmds ip pgrep pkill
	trap "tst_brkm TBROK 'test interrupted'" INT
}

cleanup()
{
	# Stop the background TCP traffic
	pkill -13 -x netstress
	tst_rhost_run -c "pkill -13 -x netstress"
}

make_background_tcp_traffic()
{
	port=$(tst_get_unused_port ipv${ipver} stream)
	netstress -R 3 -g $port > /dev/null 2>&1 &
	tst_rhost_run -b -c "netstress -l -H $(tst_ipaddr) -g $port"
}

# check_connectivity_interval [CNT] [RESTORE]
# CNT: loop step
# RESTORE: whether restore ip addr (not required, default false)
check_connectivity_interval()
{
	local cnt="$1"
	local restore="${2:-}"

	[ $CHECK_INTERVAL -eq 0 ] && return
	[ $(($cnt % $CHECK_INTERVAL)) -ne 0 ] && return

	tst_resm TINFO "check connectivity step $cnt"
	[ -n "$restore" ] && restore_ipaddr

	check_connectivity
	return $?
}
