#!/bin/sh
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
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
#
# Verify that measurements are added to the measurement list based on policy.

TST_TESTFUNC="test"
TST_SETUP="init"
TST_CNT=3
. ima_setup.sh

TEST_FILE="test.txt"
HASH_COMMAND="sha1sum"
POLICY="$IMA_DIR/policy"

init()
{
	grep -q '^CONFIG_IMA_DEFAULT_HASH_SHA256=y' /boot/config-$(uname -r) && \
		HASH_COMMAND="sha256sum"
	tst_res TINFO "detected IMA algoritm: ${HASH_COMMAND%sum}"
	tst_check_cmds $HASH_COMMAND
	[ -f "$POLICY" ] || tst_res TINFO "not using default policy"
}

ima_check()
{
	EXPECT_PASS grep -q $($HASH_COMMAND $TEST_FILE) $ASCII_MEASUREMENTS
}

test1()
{
	tst_res TINFO "verify adding record to the IMA measurement list"
	ROD echo "$(date) this is a test file" \> $TEST_FILE
	ima_check
}

test2()
{
	local device

	tst_res TINFO "verify updating record in the IMA measurement list"

	device="$(df . | sed -e 1d | cut -f1 -d ' ')"
	if grep -q $device /proc/mounts; then
		if grep -q "${device}.*ext[2-4]" /proc/mounts; then
			grep -q "${device}.*ext[2-4].*i_version" /proc/mounts || \
				tst_res TINFO "device '$device' is not mounted with iversion"
		fi
	else
		tst_res TWARN "could not find mount info for device '$device'"
	fi

	ROD echo "$(date) modified file" \> $TEST_FILE
	ima_check
}

test3()
{
	local dir="user"
	local user="nobody"

	tst_res TINFO "verify measuring user files"

	id $user >/dev/null 2>/dev/null || tst_brk TCONF "missing system user $user (wrong installation)"
	tst_check_cmds sudo

	mkdir -m 0700 $dir
	chown $user $dir
	cd $dir

	sudo -n -u $user sh -c "echo $(date) user file > $TEST_FILE;
		cat $TEST_FILE > /dev/null"

	ima_check
	cd ..
}

tst_run
