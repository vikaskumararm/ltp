#!/bin/sh
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Based on reproducer made by Ignaz Forster <iforster@suse.de>

TST_SETUP="setup"
TST_CLEANUP="cleanup"
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
}

do_test()
{
	local f

	tst_mount

	echo lower > $lower/ll.text
	echo lower > $lower/lo1.text
	echo lower > $lower/lo2.text
	echo upper > $upper/uu.text
	echo overlay > $merged/oo.text
	echo overlay > $merged/lo1.text
	echo overlay >> $merged/lo2.text

	for f in $(find $TST_MNTPOINT -type f); do
		EXPECT_PASS cat $f
	done
}

cleanup()
{
	tst_umount $TST_DEVICE

	TST_DEVICE="$device_backup"
	TST_FS_TYPE="$fs_type_backup"
	TST_MNTPOINT="$mntpoint_backup"
	TST_MNT_PARAMS="$params_backup"
}

tst_run
