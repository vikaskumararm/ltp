// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic fsopen() test which tries to configure and mount the filesystem as
 * well.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

#define TCASE_ENTRY(_flags)	{.name = "Flag " #_flags, .flags = _flags}

static struct tcase {
	char *name;
	unsigned int flags;
} tcases[] = {
	TCASE_ENTRY(0),
	TCASE_ENTRY(FSOPEN_CLOEXEC),
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fd, fsmfd;

	TEST(fd = fsopen(tst_device->fs_type, tc->flags));
	if (fd == -1) {
		tst_res(TFAIL | TERRNO, "fsopen() failed");
		return;
	}

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

	if (tst_is_mounted(MNTPOINT))
		tst_res(TPASS, "%s: fsopen() passed", tc->name);

	SAFE_UMOUNT(MNTPOINT);

out:
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = fsopen_supported_by_kernel,
	.needs_root = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.dev_fs_flags = TST_FS_SKIP_FUSE,
};
