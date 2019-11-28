// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Tests that writing to fallocated file works when filesystem is full.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "tst_test.h"
#include "lapi/fallocate.h"

#define MNTPOINT "mntpoint"
#define FALLOCATE_BLOCKS 16
#define TESTED_FLAGS "fallocate(FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE)"

static int fd;
static char *buf = NULL;

static void run(void)
{
	size_t bufsize, i;
	struct stat statbuf;

	fd = SAFE_OPEN(MNTPOINT "/test_file", O_WRONLY | O_CREAT);

	// Use real FS block size, otherwise fallocate() call will test
	// different things on different platforms
	SAFE_FSTAT(fd, &statbuf);
	bufsize = FALLOCATE_BLOCKS * statbuf.st_blksize;
	buf = realloc(buf, bufsize);

	if (!buf) {
		tst_brk(TBROK, "Buffer allocation failed");
		SAFE_CLOSE(fd);
		return;
	}

	TEST(fallocate(fd, 0, 0, bufsize));

	if (TST_RET) {
		if (errno == ENOTSUP) {
			tst_res(TCONF | TTERRNO, "fallocate() not supported");
			SAFE_CLOSE(fd);
			return;
		}

		tst_brk(TBROK | TTERRNO, "fallocate(fd, 0, 0, %i)", bufsize);
	}

	tst_fill_fs(MNTPOINT, 1);

	TEST(write(fd, buf, bufsize));

	if (TST_RET < 0)
		tst_res(TFAIL | TTERRNO, "write() failed unexpectedly");
	else if (TST_RET != bufsize)
		tst_res(TFAIL,
			"Short write(): %ld bytes (expected %zu)",
			TST_RET, bufsize);
	else
		tst_res(TPASS, "write() wrote %ld bytes", TST_RET);

	// fallocate(1 block) may pass here on XFS. Original test allocated
	// 8KB (2 blocks on x86) so keep the original behavior.
	TEST(fallocate(fd, 0, bufsize, 2 * statbuf.st_blksize));
	if (TST_RET != -1)
		tst_brk(TFAIL, "fallocate() succeeded unexpectedly");

	if (TST_ERR != ENOSPC)
		tst_brk(TFAIL | TTERRNO, "fallocate() should fail with ENOSPC");

	tst_res(TPASS | TTERRNO, "fallocate() on full FS");

	// btrfs won't release any space unless the whole file has been
	// deallocated. Original test did that, keep the original behavior.
	TEST(fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 0,
		bufsize));
	if (TST_RET == -1) {
		if (TST_ERR == ENOTSUP)
			tst_brk(TCONF, TESTED_FLAGS);

		tst_brk(TBROK | TTERRNO, TESTED_FLAGS);
	}
	tst_res(TPASS, TESTED_FLAGS);

	TEST(write(fd, buf, 10));
	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "write()");
	else
		tst_res(TPASS, "write()");

	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	if (buf)
		free(buf);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.cleanup = cleanup,
	.test_all = run,
};
