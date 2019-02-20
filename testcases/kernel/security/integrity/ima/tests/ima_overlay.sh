#!/bin/sh
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Based on reproducer made by Ignaz Forster <iforster@suse.de>

TST_SETUP="setup"
TST_CLEANUP="cleanup"
TST_NEEDS_DEVICE=1

. ima_setup.sh
TST_TESTFUNC="do_test"

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

	grep -q ima_appraise_tcb /proc/cmdline || \
		tst_brk TCONF "Test requires ima_appraise_tcb kernel parameter"
}

do_test()
{
	local file="foo.txt"
	local f

	tst_mount
	mounted=1

	ROD echo lower \> $lower/$file
	if ! echo overlay > $merged/$file 2>/dev/null; then
		tst_res TFAIL "Cannot write to merged layer"
		return
	fi

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
