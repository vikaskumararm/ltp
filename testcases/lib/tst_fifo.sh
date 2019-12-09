#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Joerg Vehlow <joerg.vehlow@aox-tech.de>

[ "$TST_NEEDS_TMPDIR" != 1 ] && tst_brk TCONF "fifo library requires TST_NEEDS_TMPDIR=1"
[ -z "$TST_TMPDIR" ] && tst_brk TCONF "TST_TMPDIR is not supposed to be empty"

export LTP_FIFO_PATH="$TST_TMPDIR"

tst_fifo_create()
{
    [ $# -ne 1 ] && tst_brk TBROK "tst_fifo_create expects 1 parameter"
    local _tst_path_req="${TST_TMPDIR}/tst_fifo_$1.req"
    local _tst_path_ack="${TST_TMPDIR}/tst_fifo_$1.ack"

    mkfifo "$_tst_path_req" || tst_brk TBROK "unable to create fifo '$_tst_path_req'"
    mkfifo "$_tst_path_ack" || tst_brk TBROK "unable to create fifo '$_tst_path_ack'"
}

tst_fifo_destroy()
{
    [ $# -ne 1 ] && tst_brk TBROK "tst_fifo_destroy expects 1 parameter"
    local _tst_path_req="${TST_TMPDIR}/tst_fifo_$1.req"
    local _tst_path_ack="${TST_TMPDIR}/tst_fifo_$1.ack"

    [ -p "$_tst_path_req" ] || tst_brk TBROK "tst_fifo_destroy fifo '$_tst_path_req' does not exist"
    [ -p "$_tst_path_ack" ] || tst_brk TBROK "tst_fifo_destroy fifo '$_tst_path_ack' does not exist"

    rm "$_tst_path_req"  || tst_brk TBROK "unable to destroy fifo '$_tst_path_req'"
    rm "$_tst_path_ack"  || tst_brk TBROK "unable to destroy fifo '$_tst_path_ack'"
}

tst_fifo_send()
{
    [ $# -ne 2 ] && tst_brk TBROK "tst_fifo_send expects 2 parameters"
    local _tst_path_req="${TST_TMPDIR}/tst_fifo_$1.req"
    local _tst_path_ack="${TST_TMPDIR}/tst_fifo_$1.ack"
    local _tst_msg="$2"

    [ -p "$_tst_path_req" ] || tst_brk TBROK "tst_fifo_send fifo '$_tst_path_req' does not exist"
    [ -p "$_tst_path_ack" ] || tst_brk TBROK "tst_fifo_send fifo '$_tst_path_ack' does not exist"

    echo -n "$_tst_msg" > "$_tst_path_req"
    cat "$_tst_path_ack" > /dev/null
}

tst_fifo_receive()
{
    [ $# -ne 1 ] && tst_brk TBROK "tst_fifo_receive expects 1 parameter"
    local _tst_path_req="${TST_TMPDIR}/tst_fifo_$1.req"
    local _tst_path_ack="${TST_TMPDIR}/tst_fifo_$1.ack"

    [ -p "$_tst_path_req" ] || tst_brk TBROK "tst_fifo_receive fifo '$_tst_path_req' does not exist"
    [ -p "$_tst_path_ack" ] || tst_brk TBROK "tst_fifo_receive fifo '$_tst_path_ack' does not exist"

    cat "$_tst_path_req"
    echo -n "OK" > "$_tst_path_ack"
}
