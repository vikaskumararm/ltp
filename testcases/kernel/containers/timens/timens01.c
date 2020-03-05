// SPDX-License-Identifier: GPL-2.0-or-later
/*

  Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>

 */
/*

  Basic test for timens_offsets error handling.

  After a call to unshare(CLONE_NEWTIME) a new timer namespace is created, the
  process that has called the unshare() can adjust offsets for CLOCK_MONOTONIC
  and CLOCK_BOOTTIME for its children by writing to the '/proc/self/timens_offsets'.

 */

#define _GNU_SOURCE
#include "lapi/setns.h"
#include "lapi/namespaces_constants.h"
#include "lapi/posix_clocks.h"
#include "tst_test.h"

static struct tcase {
	const char *offsets;
	int exp_err;
} tcases[] = {
	/* obvious garbage */
	{"not an offset", EINVAL},
	/* missing nanoseconds */
	{"1 10", EINVAL},
	/* negative nanoseconds */
	{"1 10 -10", EINVAL},
	/* nanoseconds > 1s */
	{"1 10 1000000001", EINVAL},
	/* unsupported CLOCK_REALTIME */
	{"0 10 0", EINVAL},
	/* mess on the second line */
	{"1 10 0\na", EINVAL},
	/* overflow kernel 64bit nanosecond timer */
	{"1 9223372036 0", ERANGE},
	{"1 -9223372036 0", ERANGE},
};

static void verify_ns_clock(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int fd, ret;

	SAFE_UNSHARE(CLONE_NEWTIME);

	fd = SAFE_OPEN("/proc/self/timens_offsets", O_WRONLY);
	ret = write(fd, tc->offsets, strlen(tc->offsets));

	if (ret != -1) {
		tst_res(TFAIL, "Write returned %i", ret);
		return;
	}

	if (errno != tc->exp_err) {
		tst_res(TFAIL | TERRNO, "Write should fail with %s, got:",
			tst_strerrno(tc->exp_err));
		return;
	}

	tst_res(TPASS | TERRNO, "Write offsets='%s'", tc->offsets);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_ns_clock,
	.needs_root = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TIME_NS=y"
	}
};
