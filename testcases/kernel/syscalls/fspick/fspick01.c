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

static struct tcase {
	char *name;
	unsigned int flags;
} tcases[] = {
	{"Flag FSPICK_CLOEXEC", FSPICK_CLOEXEC},
	{"Flag FSPICK_SYMLINK_NOFOLLOW", FSPICK_SYMLINK_NOFOLLOW},
	{"Flag FSPICK_NO_AUTOMOUNT", FSPICK_NO_AUTOMOUNT},
	{"Flag FSPICK_EMPTY_PATH", FSPICK_EMPTY_PATH},
};

static int ismounted;

static void cleanup(void)
{
	if (ismounted)
		SAFE_UMOUNT(MNTPOINT);
}

static void setup(void)
{
	int fd, fsmfd;

	fsopen_supported_by_kernel();

	TEST(fsopen(tst_device->fs_type, 0));
	fd = TST_RET;

	if (fd == -1)
		tst_brk(TBROK | TERRNO, "fsopen() failed");

	TEST(fsconfig(fd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_brk(TBROK | TERRNO, "fsconfig failed");
	}

	TEST(fsconfig(fd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET == -1) {
		SAFE_CLOSE(fd);
		tst_brk(TBROK | TERRNO, "fsconfig failed");
	}

	TEST(fsmount(fd, 0, 0));
	SAFE_CLOSE(fd);

	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "fsmount() failed");

	fsmfd = TST_RET;
	TEST(move_mount(fsmfd, "", AT_FDCWD, MNTPOINT,
			MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(fsmfd);

	if (TST_RET == -1)
		tst_brk(TBROK | TERRNO, "move_mount() failed");

	ismounted = 1;

	if (tst_ismount(MNTPOINT))
		tst_brk(TBROK | TERRNO, "device not mounted");
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fspick_fd;

	TEST(fspick(AT_FDCWD, MNTPOINT, tc->flags));
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
	tst_res(TPASS, "%s: fspick() passed", tc->name);
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
