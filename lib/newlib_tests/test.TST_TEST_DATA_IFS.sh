#!/bin/sh
#
# Example test with tests in a single function, using $TST_TEST_DATA and
# $TST_TEST_DATA_IFS
#

TST_TESTFUNC=do_test
TST_TEST_DATA="foo:bar:d dd"
TST_TEST_DATA_IFS=":"
. tst_test.sh

do_test()
{
	tst_res TPASS "Test $1 passed with data '$2'"
}

tst_run
# output:
# test 1 TPASS: Test 1 passed with data 'foo'
# test 2 TPASS: Test 2 passed with data 'bar'
# test 3 TPASS: Test 3 passed with data 'd dd'
