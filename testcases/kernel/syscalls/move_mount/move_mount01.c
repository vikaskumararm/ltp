// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic move_mount() test.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

static int fd, fsmfd, mmfd = -1;

static void cleanup(void)
{
	SAFE_CLOSE(fsmfd);

	if (mmfd != -1) {
		SAFE_CLOSE(mmfd);
		TEST(umount(MNTPOINT));
	}

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	TEST(fsopen(tst_device->fs_type, 0));
	fd = TST_RET;

	if (fd == -1)
		tst_brk(TBROK | TERRNO, "fsopen() failed");

	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1)
		goto out;

	TEST(fsconfig(fd, FSCONFIG_SET_FLAG, "rw", NULL, 0));
	if (TST_RET == -1)
		goto out;

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1)
		goto out;

	TEST(fsmount(fd, 0, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_brk(TBROK | TERRNO, "fsmount() failed");
	}

	fsmfd = TST_RET;
	return;

out:
	SAFE_CLOSE(fd);
	tst_brk(TBROK | TERRNO, "fsconfig() failed");
}

static void run(void)
{
	TEST(move_mount(fsmfd, "", AT_FDCWD, MNTPOINT,
			MOVE_MOUNT_F_EMPTY_PATH));
	if (TST_RET == -1)
		tst_brk(TFAIL | TERRNO, "move_mount() failed");

	mmfd = TST_RET;

	tst_res(TPASS, "move_mount() passed");
}

static struct tst_test test = {
	.min_kver = "5.2",
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
};
