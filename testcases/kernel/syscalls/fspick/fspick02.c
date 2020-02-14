// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic fspick() failure tests.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

int fd, fsmfd, mmfd;

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

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(fspick(tc->dirfd, tc->pathname, tc->flags));
	if (TST_RET != -1) {
		SAFE_CLOSE(TST_RET);
		tst_brk(TFAIL, "%s: fspick() succeeded unexpectedly (index: %d)",
			tc->name, n);
	}

	if (tc->exp_errno != TST_ERR) {
		tst_brk(TFAIL | TTERRNO, "%s: fspick() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
	}

	tst_res(TPASS | TTERRNO, "%s: fspick() failed as expected", tc->name);
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
