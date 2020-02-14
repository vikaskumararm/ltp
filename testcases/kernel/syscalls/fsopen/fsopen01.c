// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic fsopen() test which tries to configure and mount the filesystem as
 * well.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

static void run(void)
{
	int fd, fsmfd;

	TEST(fsopen(tst_device->fs_type, 0));
	fd = TST_RET;

	if (fd == -1)
		tst_brk(TFAIL | TERRNO, "fsopen() failed");

	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1) {
		tst_res(TBROK | TERRNO, "fsconfig() failed");
		goto out;
	}

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1) {
		tst_res(TBROK | TERRNO, "fsconfig() failed");
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

	SAFE_CLOSE(TST_RET);

	TEST(umount(MNTPOINT));

	tst_res(TPASS, "fsopen() passed");

out:
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.min_kver = "5.2",
	.test_all = run,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
};
