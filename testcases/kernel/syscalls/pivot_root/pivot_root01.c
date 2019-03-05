/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (c) 2019 Google, Inc.
 */

#define _GNU_SOURCE

#include <config.h>

#include <errno.h>
#include <linux/unistd.h>
#include <sched.h>

#include <sys/mount.h>
#include <stdlib.h>

#include "tst_test.h"

#ifdef HAVE_LIBCAP
#include <sys/capability.h>
#endif

static const char* chroot_dir = "chroot";
static const char* new_root = "/new_root";
static const char* put_old = "/new_root/put_old";
static const char* put_old_fs = "/put_old_fs";
static const char* put_old_bad = "/put_old_fs/put_old";

/*
 * Test consists of a series of steps that allow pivot_root to succeed, which
 * is run when param is NORMAL. All other values tweak one of the steps to
 * induce a failure, and check the errno is as expected.
 */
#define NORMAL 0

/*
 * EBUSY
 * new_root or put_old are on the current root file system
 */
#define NEW_ROOT_ON_CURRENT_ROOT 1

/*
 * EINVAL
 * put_old is not underneath new_root
 * Note: if put_old and new_root are on the same fs,
 * pivot_root fails with EBUSY before testing reachability
 */
#define PUT_OLD_NOT_UNDERNEATH_NEW_ROOT 2

/*
 * ENOTDIR
 * new_root or put_old is not a directory
 */
#define PUT_OLD_NOT_DIR 3

/*
 * EPERM
 * The calling process does not have the CAP_SYS_ADMIN capability.
 */
#define NO_CAP_SYS_ADMIN 4

static const struct test_case {
	int test_case;
	int expected_error;
} test_cases[] = {
	{NORMAL, 0},
	{NEW_ROOT_ON_CURRENT_ROOT, EBUSY},
	{PUT_OLD_NOT_UNDERNEATH_NEW_ROOT, EINVAL},
	{PUT_OLD_NOT_DIR, ENOTDIR},
	{NO_CAP_SYS_ADMIN, EPERM},
};

#ifdef HAVE_LIBCAP
static void drop_cap_sys_admin(void)
{
	cap_value_t cap_value[] = { CAP_SYS_ADMIN };
	cap_t cap = cap_get_proc();
	if (!cap)
		tst_brk(TFAIL | TERRNO, "cap_get_proc failed");

	if (cap_set_flag(cap, CAP_EFFECTIVE, 1, cap_value, CAP_CLEAR))
		tst_brk(TFAIL | TERRNO, "cap_set_flag failed");

	if (cap_set_proc(cap))
		tst_brk(TFAIL | TERRNO, "cap_set_proc failed");
}
#endif

#ifdef HAVE_UNSHARE
static void run(unsigned int test_case)
{
	/* Work in child process - needed to undo unshare and chroot */
	if (SAFE_FORK()) {
		tst_reap_children();
		return;
	}

	/* pivot_root requires no shared mounts exist in process namespace */
	TEST(unshare(CLONE_NEWNS | CLONE_FS));
	if (TST_RET == -1)
		tst_brk(TFAIL | TERRNO, "unshare failed");

	/*
	 * Create an initial root dir. pivot_root doesn't work if the initial root
	 * dir is a initramfs, so use chroot to create a safe environment
	 */
	SAFE_MOUNT("none", "/", NULL, MS_REC|MS_PRIVATE, NULL);
	SAFE_MOUNT("none", chroot_dir, "tmpfs", 0, 0);
	SAFE_CHROOT(chroot_dir);

	/* Create our new root location */
	SAFE_MKDIR(new_root, 0777);

	/*
	 * pivot_root only works if new_root is a mount point, so mount a tmpfs
	 * unless testing for that fail mode
	 */
	if (test_cases[test_case].test_case != NEW_ROOT_ON_CURRENT_ROOT)
		SAFE_MOUNT("none", new_root, "tmpfs", 0, 0);

	/*
	 * Create put_old under new_root, unless testing for that specific fail
	 * mode
	 */
	const char* actual_put_old = NULL;
	if (test_cases[test_case].test_case == PUT_OLD_NOT_UNDERNEATH_NEW_ROOT) {
		actual_put_old = put_old_bad;
		SAFE_MKDIR(put_old_fs, 0777);
		SAFE_MOUNT("none", put_old_fs, "tmpfs", 0, 0);
		SAFE_MKDIR(put_old_bad, 0777);
	} else {
		actual_put_old = put_old;

		/* put_old must be a directory for success */
		if (test_cases[test_case].test_case == PUT_OLD_NOT_DIR)
			SAFE_CREAT(put_old, 0777);
		else
			SAFE_MKDIR(put_old, 0777);
	}

	if (test_cases[test_case].test_case == NO_CAP_SYS_ADMIN)
#ifdef HAVE_LIBCAP
		drop_cap_sys_admin();
#else
		tst_brk(TCONF,
			"System doesn't have POSIX capabilities support");
#endif

	/* Test the syscall */
	TEST(syscall(__NR_pivot_root, new_root, actual_put_old));

	/* If NORMAL it should have succeeded */
	if (test_cases[test_case].test_case == NORMAL) {
		if (TST_RET) {
			tst_res(TFAIL | TERRNO, "pivot_root failed");
			exit(TBROK);
		} else {
			tst_res(TPASS, "pivot_root succeeded");
			exit(TPASS);
		}
	}

	/* pivot_root is expected to fail */
	if (TST_RET == 0) {
		tst_res(TFAIL, "pivot_root succeeded unexpectedly");
		exit(TBROK);
	}

	/* Check error code is correct */
	if (errno != test_cases[test_case].expected_error) {
		tst_res(TFAIL | TERRNO,	"pivot_root failed with wrong errno");
		exit(TBROK);
	}

	tst_res(TPASS, "pivot_root failed as expected with %s",
		strerror(errno));
	exit(TPASS);
}

#else
static void run(void)
{
	tst_brk(TCONF, NULL, "unshare is undefined.");
}
#endif

static void setup(void)
{
	SAFE_MKDIR(chroot_dir, 0777);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(test_cases),
	.needs_tmpdir = 1,
	.needs_root = 1,
	.forks_child = 1,
	.setup = setup,
};
