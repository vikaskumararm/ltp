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

static struct tcase {
	char *name;
	unsigned int flags;
} tcases[] = {
	{"Flag MOVE_MOUNT_F_SYMLINKS", MOVE_MOUNT_F_SYMLINKS},
	{"Flag MOVE_MOUNT_F_AUTOMOUNTS", MOVE_MOUNT_F_AUTOMOUNTS},
	{"Flag MOVE_MOUNT_F_EMPTY_PATH", MOVE_MOUNT_F_EMPTY_PATH},
	{"Flag MOVE_MOUNT_T_SYMLINKS", MOVE_MOUNT_T_SYMLINKS},
	{"Flag MOVE_MOUNT_T_AUTOMOUNTS", MOVE_MOUNT_T_AUTOMOUNTS},
	{"Flag MOVE_MOUNT_T_EMPTY_PATH", MOVE_MOUNT_T_EMPTY_PATH},
};

static void setup(void)
{
	fsopen_supported_by_kernel();
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fsmfd, fd;

	TEST(fsopen(tst_device->fs_type, 0));
	fd = TST_RET;

	if (fd == -1)
		tst_brk(TBROK | TERRNO, "fsopen() failed");

	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_brk(TBROK | TERRNO, "fsconfig() failed");
	}

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_brk(TBROK | TERRNO, "fsconfig() failed");
	}

	TEST(fsmount(fd, 0, 0));
	SAFE_CLOSE(fd);

	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "fsmount() failed");

	fsmfd = TST_RET;
	TEST(move_mount(fsmfd, "", AT_FDCWD, MNTPOINT,
			tc->flags | MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(fsmfd);

	if (TST_RET == -1)
		tst_brk(TFAIL | TERRNO, "move_mount() failed");

	if (!tst_ismount(MNTPOINT))
		tst_res(TPASS, "%s: move_mount() passed", tc->name);

	SAFE_UMOUNT(MNTPOINT);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.dev_fs_flags = TST_FS_SKIP_FUSE,
};
