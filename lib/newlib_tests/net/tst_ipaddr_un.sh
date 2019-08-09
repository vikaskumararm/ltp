#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_TESTFUNC="test_tst_ipaddr_un"
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="ip"
. tst_net.sh

#_tst_should_be_find_commented_out() { echo foo; }
#should_NOT_be_find_tst_foo_commented_out() { echo foo; }
_tst_should_be_find() { echo foo; }
should_NOT_be_find_tst_foo() { echo foo; }
 _tst_should_be_find_space() { echo foo; }
 should_NOT_be_find_tst_foo_space() { echo foo; }

TST_should_be_find="foo"
should_NOT_be_find_TST_foo="bar"
SHOULD_NOT_BE_FIND_TST_FOO="bar"
 TST_should_be_find_space="foo space"
 should_NOT_be_find_TST_foo_space="bar space"

test_tst_ipaddr_un()
{
	local ip ipc ipcr
	local m=$((255*255))

	# FIXME: debug
	echo "echo TST_should_not_be_found"
	echo "echo TST_SHOULD_NOT_BE_FOUND"
	echo "echo $TST_THIS_SHOULD_BE_FIND"
	_tst_should_be_find_space
	should_NOT_be_find_tst_foo
	should_NOT_be_find_tst_foo_space
	tst_res TPASS "foo $TST_should_be_find_space"
	return # FIXME: debug

	for i in \
		-1 0 1 \
		253 254 255 256 \
		509 510 511 \
		$((m-2)) $((m-1)) $m \
	; do
		ip="$(tst_ipaddr_un 1 $1)"
		ipc="$(tst_ipaddr_un -c $1)"
		ipcr="$(tst_ipaddr_un -c $1 rhost)"
		tst_res TPASS "$1, ip: '$ip', ipc: '$ipc', ipcr: '$ipcr'"
	done
}

tst_run
