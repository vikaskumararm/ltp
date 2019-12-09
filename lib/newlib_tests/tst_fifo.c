// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Joerg Vehlow <joerg.vehlow@aox-tech.de>
 */

#include "tst_test.h"
#include "tst_fifo.h"

#define MSG_P2C "AbcD"
#define MSG_C2P "Hello World"

static void do_setup(void)
{
    tst_fifo_init();
}

static void do_test(void)
{
    tst_fifo_create("p2c");
    tst_fifo_create("c2p");

    pid_t pid = SAFE_FORK();
    if (pid == 0) {
        char data[sizeof(MSG_P2C)];
        if (tst_fifo_receive("p2c", data, sizeof(data)) != strlen(MSG_P2C))
            tst_res(TFAIL, "Child did not receive expected length");
        if (strcmp(data, MSG_P2C) != 0)
            tst_res(TFAIL, "Child did not receive expected value ('%s' != '%s')", MSG_P2C, data);
        else
            tst_res(TPASS, "Child received expected value");

        tst_fifo_send("c2p", MSG_C2P);
    } else {
        tst_fifo_send("p2c", MSG_P2C);

        char data[sizeof(MSG_C2P)];
        if (tst_fifo_receive("c2p", data, sizeof(data)) != strlen(MSG_C2P))
            tst_res(TFAIL, "Parent did not receive expected length");
        if (strcmp(data, MSG_C2P) != 0)
            tst_res(TFAIL, "Parent did not receive expected value ('%s' != '%s')", MSG_C2P, data);
        else
            tst_res(TPASS, "Parent received expected value");
    }
}

static struct tst_test test = {
    .needs_tmpdir = 1,
    .forks_child = 1,
    .setup = do_setup,
	.test_all = do_test
};
