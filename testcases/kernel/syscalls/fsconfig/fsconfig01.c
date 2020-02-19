// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic fsconfig() test which tries to configure and mount the filesystem as
 * well.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

static void setup(void)
{
	fsopen_supported_by_kernel();
}

static void run(void)
{
	int fd, fsmfd;

	TEST(fsopen(tst_device->fs_type, 0));
	fd = TST_RET;

	if (fd == -1)
		tst_brk(TBROK | TERRNO, "fsopen() failed");

	TEST(fsconfig(fd, FSCONFIG_SET_FLAG, "rw", NULL, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TERRNO, "fsconfig() failed");
		goto out;
	}

	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TERRNO, "fsconfig() failed");
		goto out;
	}

	TEST(fsconfig(fd, FSCONFIG_SET_PATH, "foo", tst_device->dev, AT_FDCWD));
	if (TST_RET == -1) {
		if (TST_ERR == EOPNOTSUPP) {
			tst_res(TINFO, "fsconfig(): FSCONFIG_SET_PATH not supported");
		} else {
			tst_res(TFAIL | TERRNO, "fsconfig() failed");
			goto out;
		}
	}

	TEST(fsconfig(fd, FSCONFIG_SET_PATH_EMPTY, "foo", tst_device->dev, AT_FDCWD));
	if (TST_RET == -1) {
		if (TST_ERR == EOPNOTSUPP) {
			tst_res(TINFO, "fsconfig(): FSCONFIG_SET_PATH_EMPTY not supported");
		} else {
			tst_res(TFAIL | TERRNO, "fsconfig() failed");
			goto out;
		}
	}

	TEST(fsconfig(fd, FSCONFIG_SET_FD, "foo", NULL, 0));
	if (TST_RET == -1) {
		if (TST_ERR == EOPNOTSUPP) {
			tst_res(TINFO, "fsconfig(): FSCONFIG_SET_FD not supported");
		} else {
			tst_res(TFAIL | TERRNO, "fsconfig() failed");
			goto out;
		}
	}

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TERRNO, "fsconfig() failed");
		goto out;
	}

	TEST(fsmount(fd, 0, 0));
	if (TST_RET == -1) {
		tst_res(TBROK | TERRNO, "fsmount() failed");
		goto out;
	}

	fsmfd = TST_RET;
	TEST(move_mount(fsmfd, "", AT_FDCWD, MNTPOINT,
			MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(fsmfd);

	if (TST_RET == -1) {
		tst_res(TBROK | TERRNO, "move_mount() failed");
		goto out;
	}

	if (!tst_ismount(MNTPOINT))
		tst_res(TPASS, "fsconfig() passed");

	SAFE_UMOUNT(MNTPOINT);

out:
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.dev_fs_flags = TST_FS_SKIP_FUSE,
};
