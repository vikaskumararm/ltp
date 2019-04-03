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

	#grep -q -e ima_policy= -e ima_appraise_tcb /proc/cmdline || \
		#tst_brk TCONF "Test requires specify IMA policy as kernel parameter"
}

do_test()
{
	local file="foo.txt"
	local f

	echo "BEFORE" # FIXME: debug
	df -T $lower $upper $work $merged # FIXME: debug

	tst_mount
	mounted=1

	echo "AFTER" # FIXME: debug
	df -T $lower $upper $work $merged # FIXME: debug

	echo "before $lower/$file" # FIXME: debug
	getfattr -m . -d $lower/$file # FIXME: debug
	ROD echo lower \> $lower/$file
	echo "after $lower/$file" # FIXME: debug
	getfattr -m . -d $lower/$file # FIXME: debug

	echo "before $merged/$file" # FIXME: debug
	getfattr -m . -d $merged/$file # FIXME: debug
	if ! echo overlay > $merged/$file 2>/dev/null; then
		tst_res TFAIL "Cannot write to merged layer"
		echo "after $merged/$file" # FIXME: debug
		return
	fi
	echo "after $merged/$file" # FIXME: debug

	for f in $(find $TST_MNTPOINT -type f); do
		getfattr -m . -d $f # FIXME: debug
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
