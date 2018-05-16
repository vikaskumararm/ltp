#!/bin/sh
#
# Example test with tests in a single function, using $TST_TEST_DATA
#

TST_TESTFUNC=do_test
TST_TEST_DATA="foo:bar:d dd"
. tst_test.sh

do_test()
{
	tst_res TPASS "Test $1 passed with data '$2'"
}

tst_run
# output:
# test 1 TPASS: Test 1 passed with data 'foo:bar:d'
# test 2 TPASS: Test 2 passed with data 'foo:bar:d'
