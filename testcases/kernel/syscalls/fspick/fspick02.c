// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic fspick() failure tests.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

static struct tcase {
	char *name;
	int dirfd;
	const char *pathname;
	unsigned int flags;
	int exp_errno;
} tcases[] = {
	{"invalid-fd", -1, MNTPOINT, FSPICK_NO_AUTOMOUNT | FSPICK_CLOEXEC, EBADF},
	{"invalid-path", AT_FDCWD, "invalid", FSPICK_NO_AUTOMOUNT | FSPICK_CLOEXEC, ENOENT},
	{"invalid-flags", AT_FDCWD, MNTPOINT, 0x10, EINVAL},
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

	if (!tst_is_mounted(MNTPOINT))
		tst_brk(TBROK | TERRNO, "device not mounted");
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(fspick(tc->dirfd, tc->pathname, tc->flags));
	if (TST_RET != -1) {
		SAFE_CLOSE(TST_RET);
		tst_res(TFAIL, "%s: fspick() succeeded unexpectedly (index: %d)",
			tc->name, n);
	} else if (tc->exp_errno != TST_ERR) {
		tst_res(TFAIL | TTERRNO, "%s: fspick() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
	} else {
		tst_res(TPASS | TTERRNO, "%s: fspick() failed as expected",
			tc->name);
	}
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
