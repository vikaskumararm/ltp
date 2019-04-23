// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

/*
 * Tests basic functions and error handling of the
 * copy_file_range syscall
 *
 * 1) Copy contents of one file to another without
 *    offset or anything special -> SUCCESS
 * 2) Copy contents with offset in source file but keep
 *    the length the same so that it exeeds EOF
 *    -> SUCCESS, copy bytes until EOF
 * 3) Try to copy contents to file open as readonly
 *    -> EBADF
 * 4) Try to copy contents to file on different mounted
 *    filesystem -> EXDEV
 * 5) Try to copy contents to directory -> EISDIR
 *
 */

#include <stdlib.h>
#include "tst_test.h"
#include "copy_file_range.h"

#include <stdbool.h>

#define MNTPOINT	"mnt_point"
#define FILE_RDONL_PATH "file_rdonl"
#define FILE_DIR_PATH	"file_dir"
#define FILE_MNTED_PATH	MNTPOINT"/file_mnted"


static int fd_src;
static int fd_dest;
static int fd_rdonly;
static int fd_dir;
static int fd_mnted;

static struct tcase {
	loff_t	off_in;
	loff_t	off_out;
	size_t	length;
	int	*copy_to_fd;
	int	bytes_copied;
	int	exp_err;
} tcases[] = {
	{0,  0, strlen(CONTENT),  &fd_dest,   strlen(CONTENT),    true},
	{0, 10, strlen(CONTENT),  &fd_dest,   strlen(CONTENT),    true},
	{10, 0, strlen(CONTENT),  &fd_dest,   strlen(CONTENT)-10, true},
	{0,  0, strlen(CONTENT),  &fd_rdonly, -1, EBADF},
	{0,  0, strlen(CONTENT),  &fd_mnted,  -1, EXDEV},
	{0,  0, strlen(CONTENT),  &fd_dir,    -1, EISDIR},
};

static void verify_copy_file_range(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	loff_t off_in  = tc->off_in;
	loff_t off_out = tc->off_out;

	TEST(sys_copy_file_range(fd_src, &off_in,
			    *tc->copy_to_fd, &off_out, tc->length, 0));
	if (tc->exp_err != TST_ERR) {
		tst_res(TFAIL,
			"copy_file_range failed with %s, expected %s",
			tst_strerrno(TST_ERR), tst_strerrno(tc->exp_err));
		return;
	}

	if (tc->bytes_copied != TST_RET) {
		tst_res(TFAIL,
			"Wrong number of bytes copied. Expected %d, copied %ld",
			tc->bytes_copied, TST_RET);
		return;
	}

	tst_res(TPASS, "copy_file_range ended with %s as expected",
			tst_strerrno(tc->exp_err));
}

static void cleanup(void)
{
	if (fd_dest > 0)
		SAFE_CLOSE(fd_dest);
	if (fd_src > 0)
		SAFE_CLOSE(fd_src);
	if (fd_rdonly > 0)
		SAFE_CLOSE(fd_rdonly);
	if (fd_mnted > 0)
		SAFE_CLOSE(fd_mnted);
	if (fd_dir > 0)
		SAFE_CLOSE(fd_dir);
}

static void setup(void)
{
	SAFE_MKDIR(FILE_DIR_PATH, 0777);

	fd_dir = SAFE_OPEN(FILE_DIR_PATH, O_RDONLY | O_DIRECTORY);
	fd_mnted = SAFE_OPEN(FILE_MNTED_PATH, O_RDWR | O_CREAT, 0664);
	fd_rdonly = SAFE_OPEN(FILE_RDONL_PATH, O_RDONLY | O_CREAT, 0664);
	fd_src  = SAFE_OPEN(FILE_SRC_PATH, O_RDWR | O_CREAT, 0664);
	fd_dest = SAFE_OPEN(FILE_DEST_PATH, O_RDWR | O_CREAT, 0664);

	SAFE_WRITE(1, fd_src,  CONTENT,  strlen(CONTENT));
}

static struct tst_test test = {
	.test = verify_copy_file_range,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "4.5",
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.dev_fs_type = "ext4",
};
