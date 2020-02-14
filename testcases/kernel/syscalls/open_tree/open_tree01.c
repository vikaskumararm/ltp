// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic open_tree() test.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"
#define OT_MNTPOINT	"ot_mntpoint"

static int fd, fsmfd, mmfd;

static void cleanup(void)
{
	if (mmfd != -1)
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

	TEST(move_mount(fsmfd, "", AT_FDCWD, MNTPOINT,
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
	int otfd;

	SAFE_MKDIR(OT_MNTPOINT, 0777);

	TEST(open_tree(AT_FDCWD, MNTPOINT, OPEN_TREE_CLONE));
	if (TST_RET == -1) {
		tst_res(TFAIL | TERRNO, "open_tree() failed");
		goto out;
	}

	otfd = TST_RET;

	/* Earlier file descriptor isn't valid anymore */
	mmfd = -1;

	TEST(move_mount(otfd, "", AT_FDCWD, OT_MNTPOINT,
			MOVE_MOUNT_F_EMPTY_PATH));

	SAFE_CLOSE(otfd);

	if (TST_RET == -1) {
		tst_res(TBROK | TERRNO, "move_mount() failed");
		goto out;
	}

	SAFE_CLOSE(TST_RET);
	TEST(umount(OT_MNTPOINT));

	tst_res(TPASS, "open_tree() passed");
out:
	SAFE_RMDIR(OT_MNTPOINT);
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
