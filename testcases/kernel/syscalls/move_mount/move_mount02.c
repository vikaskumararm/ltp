// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic move_mount() failure tests.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

int fd, invalid_fd = -1, fsmfd;

static struct tcase {
	char *name;
	int *from_dirfd;
	const char *from_pathname;
	int to_dirfd;
	const char *to_pathname;
	unsigned int flags;
	int exp_errno;
} tcases[] = {
	{"invalid-from-fd", &invalid_fd, "", AT_FDCWD, MNTPOINT, MOVE_MOUNT_F_EMPTY_PATH, EBADF},
	{"invalid-from-path", &fsmfd, "invalid", AT_FDCWD, MNTPOINT, MOVE_MOUNT_F_EMPTY_PATH, ENOENT},
	{"invalid-to-fd", &fsmfd, "", -1, MNTPOINT, MOVE_MOUNT_F_EMPTY_PATH, EBADF},
	{"invalid-to-path", &fsmfd, "", AT_FDCWD, "invalid", MOVE_MOUNT_F_EMPTY_PATH, ENOENT},
	{"invalid-flags", &fsmfd, "", AT_FDCWD, MNTPOINT, 0x08, EINVAL},
};

static void cleanup(void)
{
	SAFE_CLOSE(fsmfd);
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

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(move_mount(*tc->from_dirfd, tc->from_pathname, tc->to_dirfd,
			tc->to_pathname, tc->flags));
	if (TST_RET != -1) {
		SAFE_CLOSE(TST_RET);
		TEST(umount(MNTPOINT));
		tst_brk(TFAIL, "%s: move_mount() succeeded unexpectedly (index: %d)",
			tc->name, n);
	}

	if (tc->exp_errno != TST_ERR) {
		tst_brk(TFAIL | TTERRNO, "%s: move_mount() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
	}

	tst_res(TPASS | TTERRNO, "%s: move_mount() failed as expected", tc->name);
}

static struct tst_test test = {
	.min_kver = "5.2",
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.format_device = 1,
	.mntpoint = MNTPOINT,
};
