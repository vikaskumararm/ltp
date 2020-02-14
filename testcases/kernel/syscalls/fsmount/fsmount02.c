// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic fsmount() failure tests.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

int fd, invalid_fd = -1;

static struct tcase {
	char *name;
	int *fd;
	unsigned int flags;
	unsigned int mount_attrs;
	int exp_errno;
} tcases[] = {
	{"invalid-fd", &invalid_fd, FSMOUNT_CLOEXEC, 0, EBADF},
	{"invalid-flags", &fd, 0x02, 0, EINVAL},
	{"invalid-attrs", &fd, FSMOUNT_CLOEXEC, 0x100, EINVAL},
};

static void cleanup(void)
{
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
	if (TST_RET != -1)
		return;

out:
	cleanup();
	tst_brk(TBROK | TERRNO, "fsconfig() failed");
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(fsmount(*tc->fd, tc->flags, tc->mount_attrs));
	if (TST_RET != -1) {
		SAFE_CLOSE(TST_RET);
		tst_brk(TFAIL, "%s: fsmount() succeeded unexpectedly (index: %d)",
			tc->name, n);
	}

	if (tc->exp_errno != TST_ERR) {
		tst_brk(TFAIL | TTERRNO, "%s: fsmount() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
	}

	tst_res(TPASS | TTERRNO, "%s: fsmount() failed as expected", tc->name);
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
};
