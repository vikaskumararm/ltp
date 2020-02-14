// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Description:
 * Basic fsopen() failure tests.
 */
#include "tst_test.h"
#include "lapi/fsmount.h"

const char *invalid_fs = "invalid";
const char *valid_fs;

static struct tcase {
	char *name;
	const char **fs;
	unsigned int flags;
	int exp_errno;
} tcases[] = {
	{"invalid-fs", &invalid_fs, 0, ENODEV},
	{"invalid-flags", &valid_fs, 0x10, EINVAL},
};

static void setup(void)
{
	valid_fs = tst_device->fs_type;
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(fsopen(*tc->fs, tc->flags));

	if (TST_RET != -1) {
		SAFE_CLOSE(TST_RET);
		tst_brk(TFAIL, "%s: fsopen() succeeded unexpectedly (index: %d)",
			tc->name, n);
	}

	if (tc->exp_errno != TST_ERR) {
		tst_brk(TFAIL | TTERRNO, "%s: fsopen() should fail with %s",
			tc->name, tst_strerrno(tc->exp_errno));
	}

	tst_res(TPASS | TTERRNO, "%s: fsopen() failed as expected", tc->name);
}

static struct tst_test test = {
	.min_kver = "5.2",
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.needs_root = 1,
	.needs_device = 1,
};
