// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 SUSE.  All Rights Reserved.
 *
 * Started by Jan Kara <jack@suse.cz>
 *
 * DESCRIPTION
 *     Check that fanotify properly merges ignore mask of an inode and
 *     mountpoint.
 *
 * This is a regression test for:
 *
 *  commit 8edc6e1688fc8f02c8c1f53a2ec4928cb1055f4d
 *  Author: Jan Kara <jack@suse.cz>
 *  Date:   Thu Nov 13 15:19:33 2014 -0800
 *
 *      fanotify: fix notification of groups with inode & mount marks
 */
#define _GNU_SOURCE
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include "tst_test.h"
#include "fanotify.h"

#if defined(HAVE_SYS_FANOTIFY_H)
#include <sys/fanotify.h>

#define EVENT_MAX 1024
/* size of the event structure, not counting name */
#define EVENT_SIZE  (sizeof (struct fanotify_event_metadata))
/* reasonable guess as to size of 1024 events */
#define EVENT_BUF_LEN        (EVENT_MAX * EVENT_SIZE)

unsigned int fanotify_prio[] = {
	FAN_CLASS_PRE_CONTENT,
	FAN_CLASS_CONTENT,
	FAN_CLASS_NOTIF
};
#define FANOTIFY_PRIORITIES ARRAY_SIZE(fanotify_prio)

#define GROUPS_PER_PRIO 3

#define BUF_SIZE 256
static char fname[BUF_SIZE];
static int fd_notify[FANOTIFY_PRIORITIES][GROUPS_PER_PRIO];

static char event_buf[EVENT_BUF_LEN];

#define MNTPOINT "mntpoint"
#define OVL_LOWER "ovl-lower"
#define OVL_UPPER "ovl-upper"
#define OVL_WORK "ovl-work"
#define OVL_MNT "ovl-mnt"

static int mount_created;
static int ovl_mounted;

static struct tcase {
	const char *tname;
	const char *mnt;
	int use_overlay;
} tcases[] = {
	{ "Fanotify merge mount mark", MNTPOINT, 0 },
	{ "Fanotify merge overlayfs mount mark", OVL_MNT, 1 },
};

static void create_fanotify_groups(void)
{
	unsigned int p, i;
	int ret;

	for (p = 0; p < FANOTIFY_PRIORITIES; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			fd_notify[p][i] = SAFE_FANOTIFY_INIT(fanotify_prio[p] |
							     FAN_NONBLOCK,
							     O_RDONLY);

			/* Add mount mark for each group */
			ret = fanotify_mark(fd_notify[p][i],
					    FAN_MARK_ADD | FAN_MARK_MOUNT,
					    FAN_MODIFY,
					    AT_FDCWD, fname);
			if (ret < 0) {
				tst_brk(TBROK | TERRNO,
					"fanotify_mark(%d, FAN_MARK_ADD | "
					"FAN_MARK_MOUNT, FAN_MODIFY, AT_FDCWD,"
					" %s) failed", fd_notify[p][i], fname);
			}
			/* Add ignore mark for groups with higher priority */
			if (p == 0)
				continue;
			ret = fanotify_mark(fd_notify[p][i],
					    FAN_MARK_ADD |
					    FAN_MARK_IGNORED_MASK |
					    FAN_MARK_IGNORED_SURV_MODIFY,
					    FAN_MODIFY, AT_FDCWD, fname);
			if (ret < 0) {
				tst_brk(TBROK | TERRNO,
					"fanotify_mark(%d, FAN_MARK_ADD | "
					"FAN_MARK_IGNORED_MASK | "
					"FAN_MARK_IGNORED_SURV_MODIFY, "
					"FAN_MODIFY, AT_FDCWD, %s) failed",
					fd_notify[p][i], fname);
			}
		}
	}
}

static void cleanup_fanotify_groups(void)
{
	unsigned int i, p;

	for (p = 0; p < FANOTIFY_PRIORITIES; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			if (fd_notify[p][i] > 0)
				SAFE_CLOSE(fd_notify[p][i]);
		}
	}
}

static void verify_event(int group, struct fanotify_event_metadata *event)
{
	if (event->mask != FAN_MODIFY) {
		tst_res(TFAIL, "group %d got event: mask %llx (expected %llx) "
			"pid=%u fd=%d", group, (unsigned long long)event->mask,
			(unsigned long long)FAN_MODIFY,
			(unsigned)event->pid, event->fd);
	} else if (event->pid != getpid()) {
		tst_res(TFAIL, "group %d got event: mask %llx pid=%u "
			"(expected %u) fd=%d", group,
			(unsigned long long)event->mask, (unsigned)event->pid,
			(unsigned)getpid(), event->fd);
	} else {
		tst_res(TPASS, "group %d got event: mask %llx pid=%u fd=%d",
			group, (unsigned long long)event->mask,
			(unsigned)event->pid, event->fd);
	}
}

void test_fanotify(unsigned int n)
{
	int ret;
	unsigned int p, i;
	struct fanotify_event_metadata *event;
	struct tcase *tc = &tcases[n];

	tst_res(TINFO, "Test #%d: %s", n, tc->tname);

	if (tc->use_overlay && !ovl_mounted) {
		tst_res(TCONF,
		        "overlayfs is not configured in this kernel.");
		return;
	}

	sprintf(fname, "%s/tfile_%d", tc->mnt, getpid());
	SAFE_TOUCH(fname, 0644, NULL);

	create_fanotify_groups();

	/*
	 * generate sequence of events
	 */
	SAFE_FILE_PRINTF(fname, "1");

	/* First verify all groups without ignore mask got the event */
	for (i = 0; i < GROUPS_PER_PRIO; i++) {
		ret = read(fd_notify[0][i], event_buf, EVENT_BUF_LEN);
		if (ret < 0) {
			if (errno == EAGAIN) {
				tst_res(TFAIL, "group %d did not get "
					"event", i);
			}
			tst_brk(TBROK | TERRNO,
				"reading fanotify events failed");
		}
		if (ret < (int)FAN_EVENT_METADATA_LEN) {
			tst_brk(TBROK,
				"short read when reading fanotify "
				"events (%d < %d)", ret,
				(int)EVENT_BUF_LEN);
		}
		event = (struct fanotify_event_metadata *)event_buf;
		if (ret > (int)event->event_len) {
			tst_res(TFAIL, "group %d got more than one "
				"event (%d > %d)", i, ret,
				event->event_len);
		} else {
			verify_event(i, event);
		}
		if (event->fd != FAN_NOFD)
			SAFE_CLOSE(event->fd);
	}
	for (p = 1; p < FANOTIFY_PRIORITIES; p++) {
		for (i = 0; i < GROUPS_PER_PRIO; i++) {
			ret = read(fd_notify[p][i], event_buf, EVENT_BUF_LEN);
			if (ret > 0) {
				tst_res(TFAIL, "group %d got event",
					p*GROUPS_PER_PRIO + i);
				if (event->fd != FAN_NOFD)
					SAFE_CLOSE(event->fd);
			} else if (ret == 0) {
				tst_brk(TBROK, "zero length "
					"read from fanotify fd");
			} else if (errno != EAGAIN) {
				tst_brk(TBROK | TERRNO,
					"reading fanotify events failed");
			} else {
				tst_res(TPASS, "group %d got no event",
					p*GROUPS_PER_PRIO + i);
			}
		}
	}
	cleanup_fanotify_groups();
}

static void setup_overlay(void)
{
	int ret;

	SAFE_MKDIR(OVL_LOWER, 0755);
	SAFE_MKDIR(OVL_UPPER, 0755);
	SAFE_MKDIR(OVL_WORK, 0755);
	SAFE_MKDIR(OVL_MNT, 0755);
	ret = mount("overlay", OVL_MNT, "overlay", 0, "lowerdir="OVL_LOWER
		    ",upperdir="OVL_UPPER",workdir="OVL_WORK);
	if (ret < 0) {
		if (errno == ENODEV) {
			tst_res(TINFO,
				"overlayfs is not configured in this kernel.");
			return;
		}
		tst_brk(TBROK | TERRNO, "overlayfs mount failed");
	}
	ovl_mounted = 1;
}

static void setup(void)
{
	SAFE_MKDIR(MNTPOINT, 0755);
	SAFE_MOUNT(MNTPOINT, MNTPOINT, "none", MS_BIND, NULL);
	mount_created = 1;

	setup_overlay();
}

static void cleanup(void)
{
	cleanup_fanotify_groups();

	if (ovl_mounted)
		SAFE_UMOUNT(OVL_MNT);

	if (mount_created)
		SAFE_UMOUNT(MNTPOINT);
}

static struct tst_test test = {
	.test = test_fanotify,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.needs_root = 1
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
