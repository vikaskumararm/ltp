// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 ARM Embedded Technologies Private Ltd. All rights reserved.
 * Author: Vikas Kumar <vikas.kumar2@arm.com>
 *
 * Use new Linux AIO API io_uring_*().
 */

#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "config.h"
#include "tst_test.h"
#include "lapi/io_uring.h"

#define DEPTH	1

struct io_uring_params param_iouring;
int fd, ret;
struct iovec iovecs;
void *iov_base_addr;
int iov_size = 64;

static struct tcase {
	int test_count;
	int flag;
} tcases[] = {
	{1, IORING_REGISTER_BUFFERS},
};

static void setup(void)
{
	memset(&param_iouring, 0, sizeof(param_iouring));

	param_iouring.flags |= IORING_SETUP_IOPOLL;

	fd = io_uring_setup(DEPTH, &param_iouring);
	if (!fd)
		tst_brk(TBROK | TERRNO, "io_uring_setup() returned %d", fd);

	iov_base_addr = malloc(sizeof(iov_size));

	iovecs.iov_base = iov_base_addr;
	iovecs.iov_len = iov_size;
}

static void cleanup(void)
{
	io_uring_register(fd, IORING_UNREGISTER_BUFFERS, NULL, DEPTH);
	memset(&param_iouring, 0, sizeof(param_iouring));
	free(iov_base_addr);
}

static void verify_io_submit(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(io_uring_register(fd, tc->flag, &iovecs, DEPTH));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "io_uring_register() returned %d", ret);
		return;
	}

	tst_res(TPASS, "io_uring_register() return %d",ret);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_io_submit,
	.tcnt = ARRAY_SIZE(tcases),
};
