#!/bin/sh
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Based on reproducer made by Ignaz Forster <iforster@suse.de>

# TODO: add ima check!
TST_NEEDS_TMPDIR=1
TST_SETUP="setup"
TST_CLEANUP="cleanup"
TST_TESTFUNC="do_test"
TST_DEVICE="overlay"
TST_FS_TYPE="overlay"
. tst_test.sh

setup()
{

	lower="$TST_MNTPOINT/lower"
	upper="$TST_MNTPOINT/upper"
	work="$TST_MNTPOINT/work"
	merged="$TST_MNTPOINT/merged"
	mkdir -p $lower $upper $work $merged

	TST_MNT_PARAMS="-o lowerdir=$lower,upperdir=$upper,workdir=$TST_MNTPOINT/work"
	TST_MNTPOINT="$merged"
}

do_test()
{
	tst_mount
	echo lower > $lower/ll.text
	echo lower > $lower/lo1.text
	echo lower > $lower/lo2.text
	echo upper > $upper/uu.text
	echo overlay > $merged/oo.text
	echo overlay > $merged/lo1.text
	echo overlay >> $merged/lo2.text

	EXPECT_PASS find $TST_MNTPOINT -type f -exec cat {} \;
}

cleanup()
{
	tst_umount $TST_DEVICE
}

tst_run
