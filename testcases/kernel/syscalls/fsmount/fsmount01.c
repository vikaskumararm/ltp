// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic fsmount() test.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

static int fd, fsmfd;

static void setup(void)
{
	TEST(fsopen(tst_device->fs_type, 0));
	fd = TST_RET;

	if (fd == -1)
		tst_brk(TBROK | TERRNO, "fsopen() failed");
}

static void cleanup(void)
{
	SAFE_CLOSE(fd);
}

static void run(void)
{
	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "fsconfig() failed");

	TEST(fsconfig(fd, FSCONFIG_SET_FLAG, "rw", NULL, 0));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "fsconfig() failed");

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "fsconfig() failed");

	TEST(fsmount(fd, 0, 0));
	if (TST_RET == -1)
		tst_brk(TFAIL | TERRNO, "fsmount() failed");

	fsmfd = TST_RET;

	TEST(move_mount(fsmfd, "", AT_FDCWD, MNTPOINT,
			MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(fsmfd);

	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "move_mount() failed");

	SAFE_CLOSE(TST_RET);
	TEST(umount(MNTPOINT));

	tst_res(TPASS, "fsmount() passed");
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
