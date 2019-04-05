#!/bin/sh
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Based on reproducer and further discussion with Ignaz Forster <iforster@suse.de>

TST_SETUP="setup"
TST_CLEANUP="cleanup"
TST_NEEDS_DEVICE=1
TST_CNT=4
. ima_setup.sh

setup()
{
	lower="$TST_MNTPOINT/lower"
	upper="$TST_MNTPOINT/upper"
	work="$TST_MNTPOINT/work"
	merged="$TST_MNTPOINT/merged"
	mkdir -p $lower $upper $work $merged

	device_backup="$TST_DEVICE"
	TST_DEVICE="overlay"

	fs_type_backup="$TST_FS_TYPE"
	TST_FS_TYPE="overlay"

	mntpoint_backup="$TST_MNTPOINT"
	TST_MNTPOINT="$merged"

	params_backup="$TST_MNT_PARAMS"
	TST_MNT_PARAMS="-o lowerdir=$lower,upperdir=$upper,workdir=$work"

	grep -q -e ima_policy= -e ima_appraise_tcb /proc/cmdline || \
		tst_brk TCONF "Test requires specify IMA policy as kernel parameter"

	tst_mount
	mounted=1
}

test1()
{
	local file="foo1.txt"

	tst_res TINFO "overwrite file in overlay"
	ROD echo lower \> $lower/$file
	EXPECT_PASS echo overlay \> $merged/$file
}

test2()
{
	local file="foo2.txt"

	tst_res TINFO "append file in overlay"
	ROD echo lower \> $lower/$file
	EXPECT_PASS echo overlay \>\> $merged/$file
}

test3()
{
	local file="foo3.txt"

	tst_res TINFO "create a new file in overlay"
	EXPECT_PASS echo overlay \> $merged/$file
}

test4()
{
	local f

	tst_res TINFO "read all created files"
	for f in $(find $TST_MNTPOINT -type f); do
		EXPECT_PASS cat $f \> /dev/null 2\> /dev/null
	done
}

cleanup()
{
	[ -n "$mounted" ] || return 0

	tst_umount $TST_DEVICE

	TST_DEVICE="$device_backup"
	TST_FS_TYPE="$fs_type_backup"
	TST_MNTPOINT="$mntpoint_backup"
	TST_MNT_PARAMS="$params_backup"
}

tst_run
