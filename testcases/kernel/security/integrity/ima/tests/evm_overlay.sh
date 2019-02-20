#!/bin/sh
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Based on reproducer and further discussion with Ignaz Forster <iforster@suse.de>

TST_SETUP="setup"
TST_CLEANUP="cleanup"
TST_NEEDS_DEVICE=1
TST_CNT=4
. ima_setup.sh

# openSUSE: always 1 in /sys/kernel/security/evm, EVM keys
# Debian: default 0 in /sys/kernel/security/evm, no EVM keys
setup()
{
	EVM_FILE="/sys/kernel/security/evm"

	[ -f "$EVM_FILE" ] || tst_brk TCONF "EVM not enabled in kernel"
	[ $(cat $EVM_FILE) -eq 1 ] || tst_brk TCONF "EVM not enabled for this boot"

	grep -q -e ima_policy= -e ima_appraise_tcb /proc/cmdline || \
		tst_brk TCONF "Test requires specify IMA policy as kernel parameter"

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

	tst_mount
	mounted=1
}

# probably the only one which can be tested without:
# => requires dmesg |grep -q "evm: key initialized"
test1()
{
	local file="foo1.txt"

	tst_res TINFO "overwrite file in overlay"
	EXPECT_PASS echo lower \> $lower/$file
	tst_res TINFO "sleep"
	sleep 1
	EXPECT_PASS echo overlay \> $merged/$file # on both don't require evm=1 to fail on bot patched and unpatched kernel
	getfattr -m . -d $lower/$file # FIXME: debug
	getfattr -m . -d $merged/$file # FIXME: debug
}

test2()
{
	local file="foo2.txt"

	tst_res TINFO "append file in overlay"
	EXPECT_PASS echo lower \> $lower/$file
	EXPECT_PASS echo overlay \>\> $merged/$file
	getfattr -m . -d $lower/$file # FIXME: debug
	getfattr -m . -d $merged/$file # FIXME: debug
}

test3()
{
	local file="foo3.txt"

	tst_res TINFO "create a new file in overlay"
	EXPECT_PASS echo overlay \> $merged/$file
	getfattr -m . -d $merged/$file # FIXME: debug
}

# fails when EVM keys presented and 1 /sys/kernel/security/evm (on openSUSE, no way to set it to 0)
# evm_overlay 4 TFAIL: cat mntpoint/merged/foo3.txt > /dev/null 2> /dev/null failed unexpectedly
# when no EVM keys (Debian, no matter whether 0 or 1 in /sys/kernel/security/evm): OK
# => requires dmesg |grep -q "evm: key initialized"
test4()
{
	local f

	tst_res TINFO "read all created files"
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
