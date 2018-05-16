#!/bin/sh
#
# Example test with tests in a single function, using $TST_TEST_DATA and $TST_CNT
#

TST_TESTFUNC=do_test
TST_CNT=2
TST_TEST_DATA="foo:bar:d dd"
. tst_test.sh

do_test()
{
	case $1 in
	1) tst_res TPASS "Test $1 passed with data '$2'";;
	2) tst_res TPASS "Test $1 passed with data '$2'";;
	esac
}

tst_run
# output:
# test 1 TPASS: Test 1 passed with data 'foo:bar:d'
# test 2 TPASS: Test 2 passed with data 'foo:bar:d'
# test 3 TPASS: Test 1 passed with data 'dd'
# test 4 TPASS: Test 2 passed with data 'dd'
