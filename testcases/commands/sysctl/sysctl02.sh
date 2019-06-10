#!/bin/sh

# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
# Author: Yang Xu<xuyang2018.jy@cn.fujitsu.com>
#
# Description:
# This is a regression test for handling overflow for file-max.
#
# when writing 2^64 to /proc/sys/fs/file-max. It will overflow
# and be set to 0. It crash system quickly.
#
# The kernel bug has been fixed in kernel:
# '7f2923c4f' (sysctl: handle overflow in proc_get_long)
# the permitted max value is  2^64-1.
# '32a5ad9c2' (sysctl: handle overflow for file-max)
# the permitted max value is 2^63-1
#
# After merged this patchset, if we exceed the max value, it will
# keep old value. Unfortunately, it introudced a new bug when set it
# to 0 and it will lead to system crash.
# This bug has been fixed by commit 9002b2146
# (kernel/sysctl.c: fix out-of-bounds access when setting file-max)

TST_TESTFUNC=do_test
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_CNT=4
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="sysctl"
dir="/proc/sys/fs/"
syms_file="/proc/kallsyms"
name="file-max"
orig_value=200000

. tst_test.sh

setup()
{
	[ ! -f "$dir""$name" ] && tst_brk TCONF \
		"$name was not supported"
	orig_value=$(cat "$dir""$name")
}

do_test()
{
	case $1 in
	1)sysctl_test_overflow 18446744073709551616;;
	2)sysctl_test_overflow 18446744073709551615;;
	3)sysctl_test_overflow 9223372036854775808;;
	4)sysctl_test_zero;;
	esac
}

sysctl_test_overflow()
{
	local old_value=$(cat "$dir""$name")

	sysctl -w "fs.file-max"=$1 >/dev/null 2>&1

	local test_value=$(cat "$dir""$name")

	echo ${test_value} |grep -q ${old_value}
	if [ $? -eq 0 ]; then
		tst_res TPASS "file-max overflow, reject it and keep old value."
	else
		tst_res TFAIL "file-max overflow and set it to ${test_value}."
	fi
	cleanup
}

sysctl_test_zero()
{
	sysctl -w "fs.file-max"=0 >/dev/null 2>&1
	[ ! -f "$syms_file" ] && tst_brk TCONF \
		"$syms_file was not supported"
	cat $syms_file  |grep kasan_report >/dev/null 2>&1
	if [ $? -eq 0 ]; then
		dmesg | grep "KASAN: global-out-of-bounds in __do_proc_doulongvec_minmax" >/dev/null
		if [ $? -eq 0 ]; then
			tst_res TFAIL "file-max is set 0 and trigger a KASAN error"
		else
			tst_res TPASS \
				"file-max is set 0 and doesn't trigger a KASAN error"
		fi
	else
		tst_res TCONF "kernel doesn't support KASAN"
	fi
}

cleanup()
{
	sysctl -w "fs.""$name"=${orig_value} >/dev/null 2>&1
}

tst_run
