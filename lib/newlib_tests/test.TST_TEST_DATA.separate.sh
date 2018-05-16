#!/bin/sh
#
# Example test with tests in separate functions, using $TST_TEST_DATA and $TST_CNT
#

TST_TESTFUNC=test
TST_CNT=2
TST_TEST_DATA="foo:bar:d dd"
. tst_test.sh

test1()
{
	tst_res TPASS "Test $1 passed with data '$2'"
}

test2()
{
	tst_res TPASS "Test $1 passed with data '$2'"
}

tst_run
# output:
# test 1 TPASS: Test 1 passed with data 'foo'
# test 2 TPASS: Test 2 passed with data 'bar'
# test 3 TPASS: Test 3 passed with data 'd dd'
