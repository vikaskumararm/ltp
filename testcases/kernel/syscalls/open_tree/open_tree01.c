// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic open_tree() test.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"
#define OT_MNTPOINT	"ot_mntpoint"

#define TCASE_ENTRY(_flags)	{.name = "Flag " #_flags, .flags = _flags}

static struct tcase {
	char *name;
	unsigned int flags;
} tcases[] = {
	TCASE_ENTRY(OPEN_TREE_CLONE),
	TCASE_ENTRY(OPEN_TREE_CLOEXEC)
};

static int dir_created;

static void setup(void)
{
	fsopen_supported_by_kernel();

	SAFE_MKDIR(OT_MNTPOINT, 0777);
	dir_created = 1;
}

static void cleanup(void)
{
	if (dir_created)
		SAFE_RMDIR(OT_MNTPOINT);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fd, fsmfd, otfd;

	TEST(fsopen(tst_device->fs_type, 0));
	fd = TST_RET;

	if (fd == -1) {
		tst_res(TBROK | TERRNO, "fsopen() failed");
		return;
	}

	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_res(TBROK | TERRNO, "fsconfig() failed");
		return;
	}

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_res(TBROK | TERRNO, "fsconfig() failed");
		return;
	}

	TEST(fsmfd = fsmount(fd, 0, 0));
	SAFE_CLOSE(fd);

	if (TST_RET == -1) {
		tst_res(TBROK | TERRNO, "fsmount() failed");
		return;
	}

	TEST(move_mount(fsmfd, "", AT_FDCWD, MNTPOINT,
			MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(fsmfd);

	if (TST_RET == -1) {
		tst_res(TBROK | TERRNO, "move_mount() failed");
		return;
	}

	if (!tst_is_mounted(MNTPOINT)) {
		tst_res(TBROK | TERRNO, "device not mounted");
		goto out;
	}

	TEST(open_tree(AT_FDCWD, MNTPOINT, tc->flags | OPEN_TREE_CLONE));
	if (TST_RET == -1) {
		tst_res(TFAIL | TERRNO, "open_tree() failed");
		goto out;
	}

	otfd = TST_RET;
	TEST(move_mount(otfd, "", AT_FDCWD, OT_MNTPOINT,
			MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(otfd);

	if (TST_RET == -1) {
		tst_res(TBROK | TERRNO, "move_mount() failed");
		goto out;
	}

	if (tst_is_mounted(OT_MNTPOINT))
		tst_res(TPASS, "%s: open_tree() passed", tc->name);

	SAFE_UMOUNT(OT_MNTPOINT);
out:
	SAFE_UMOUNT(MNTPOINT);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.dev_fs_flags = TST_FS_SKIP_FUSE,
};
