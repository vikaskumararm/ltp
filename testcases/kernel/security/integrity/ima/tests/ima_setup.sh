#!/bin/sh
# Copyright (c) 2009 IBM Corporation
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
# Author: Mimi Zohar, zohar@ibm.vnet.ibm.com
# Author: Petr Vorel <pvorel@suse.cz>

TST_SETUP="setup"
TST_CLEANUP="cleanup"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="cut"
. tst_test.sh

UMOUNT=

mount_helper()
{
	local type="$1"
	local default_dir="$2"
	local dir

	grep ^$type /proc/mounts | cut -d ' ' -f2 && return

	if ! mkdir -p $default_dir; then
		tst_brk TBROK "Failed to create $default_dir"
	fi
	if ! mount -t $type $type $default_dir; then
		tst_brk TBROK "Failed to mount $type"
	fi
	UMOUNT="$default_dir $UMOUNT"
	echo $default_dir
}

setup()
{
	SYSFS="$(mount_helper sysfs /sys)"
	SECURITYFS="$(mount_helper securityfs $SYSFS/kernel/security)"

	IMA_DIR="$SECURITYFS/ima"
	[ -d "$IMA_DIR" ] || tst_brk TCONF "IMA not enabled in kernel"
}

cleanup()
{
	local dir
	for dir in $UMOUNT; do
		umount $UMOUNT
	done
}
