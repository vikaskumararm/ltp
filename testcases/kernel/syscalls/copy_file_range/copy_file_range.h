// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

#ifndef __COPY_FILE_RANGE_H__
#define __COPY_FILE_RANGE_H__

#include "lapi/syscalls.h"

#define SUCCESS		0

#define MNTPOINT	"mnt_point"
#define FILE_SRC_PATH   "file_src"
#define FILE_DEST_PATH  "file_dest"
#define FILE_RDONL_PATH "file_rdonl"
#define FILE_DIR_PATH	"file_dir"
#define FILE_MNTED_PATH	MNTPOINT"/file_mnted"


#define CONTENT "ABCDEFGHIJKLMNOPQRSTUVWXYZ12345\n"

static int sys_copy_file_range(int fd_in, loff_t *off_in,
		int fd_out, loff_t *off_out, size_t len, unsigned int flags)
{
	return tst_syscall(__NR_copy_file_range, fd_in, off_in, fd_out,
			off_out, len, flags);
}

#endif /* __COPY_FILE_RANGE_H__ */
