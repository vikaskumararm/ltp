// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic fspick() test.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

static int fd, fsmfd, mmfd;

static void cleanup(void)
{
	SAFE_CLOSE(mmfd);
	SAFE_CLOSE(fsmfd);
	TEST(umount(MNTPOINT));
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	char *err = "fsconfig()";

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
		err = "fsmount()";
		goto out;
	}

	fsmfd = TST_RET;

	TEST(move_mount(TST_RET, "", AT_FDCWD, MNTPOINT,
			MOVE_MOUNT_F_EMPTY_PATH));
	if (TST_RET != -1) {
		mmfd = TST_RET;
		return;
	}

	SAFE_CLOSE(fsmfd);

	err = "move_mount()";

out:
	SAFE_CLOSE(fd);
	tst_brk(TBROK | TERRNO, "%s failed", err);
}

static void run(void)
{
	int fspick_fd;

	TEST(fspick(AT_FDCWD, MNTPOINT, FSPICK_NO_AUTOMOUNT | FSPICK_CLOEXEC));
	if (TST_RET == -1)
		tst_brk(TFAIL | TERRNO, "fspick() failed");

	fspick_fd = TST_RET;

	TEST(fsconfig(fspick_fd, FSCONFIG_SET_STRING, "user_xattr", "false", 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fspick_fd);
		tst_brk(TBROK | TERRNO, "fsconfig() failed");
	}

	TEST(fsconfig(fspick_fd, FSCONFIG_SET_FLAG, "ro", NULL, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fspick_fd);
		tst_brk(TBROK | TERRNO, "fsconfig() failed");
	}

	SAFE_CLOSE(fspick_fd);
	tst_res(TPASS, "fspick() passed");
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
