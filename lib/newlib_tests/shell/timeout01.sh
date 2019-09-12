#!/bin/sh

TST_TESTFUNC=do_test

TST_TIMEOUT=-1
. tst_test.sh

do_test()
{
    tst_res TPASS "timeout $TST_TIMEOUT set"
}

tst_run
