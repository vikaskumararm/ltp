// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Red Hat, Inc.  All rights reserved.
 * Author: Zorro Lang <zlang@redhat.com>
 *
 * Description:
 * Basic fsmount() test.
 */

#include "tst_test.h"
#include "lapi/fsmount.h"

#define MNTPOINT	"mntpoint"

static struct tcase {
	char *name;
	unsigned int flags;
	unsigned int mount_attrs;
} tcases[] = {
	{"Flag 0, attr RDONLY", 0, MOUNT_ATTR_RDONLY},
	{"Flag 0, attr NOSUID", 0, MOUNT_ATTR_NOSUID},
	{"Flag 0, attr NODEV", 0, MOUNT_ATTR_NODEV},
	{"Flag 0, attr NOEXEC", 0, MOUNT_ATTR_NOEXEC},
	{"Flag 0, attr RELATIME", 0, MOUNT_ATTR_RELATIME},
	{"Flag 0, attr NOATIME", 0, MOUNT_ATTR_NOATIME},
	{"Flag 0, attr STRICTATIME", 0, MOUNT_ATTR_STRICTATIME},
	{"Flag 0, attr NODIRATIME", 0, MOUNT_ATTR_NODIRATIME},
	{"Flag CLOEXEC, attr RDONLY", FSMOUNT_CLOEXEC, MOUNT_ATTR_RDONLY},
	{"Flag CLOEXEC, attr NOSUID", FSMOUNT_CLOEXEC, MOUNT_ATTR_NOSUID},
	{"Flag CLOEXEC, attr NODEV", FSMOUNT_CLOEXEC, MOUNT_ATTR_NODEV},
	{"Flag CLOEXEC, attr NOEXEC", FSMOUNT_CLOEXEC, MOUNT_ATTR_NOEXEC},
	{"Flag CLOEXEC, attr RELATIME", FSMOUNT_CLOEXEC, MOUNT_ATTR_RELATIME},
	{"Flag CLOEXEC, attr NOATIME", FSMOUNT_CLOEXEC, MOUNT_ATTR_NOATIME},
	{"Flag CLOEXEC, attr STRICTATIME", FSMOUNT_CLOEXEC, MOUNT_ATTR_STRICTATIME},
	{"Flag CLOEXEC, attr NODIRATIME", FSMOUNT_CLOEXEC, MOUNT_ATTR_NODIRATIME},
};

static void setup(void)
{
	fsopen_supported_by_kernel();
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	int sfd, mfd;

	TEST(fsopen(tst_device->fs_type, FSOPEN_CLOEXEC));
	if (TST_RET == -1) {
		tst_brk(TBROK | TTERRNO, "fsopen() on %s failed",
			tst_device->fs_type);
	}
	sfd = TST_RET;

	TEST(fsconfig(sfd, FSCONFIG_SET_STRING, "source", tst_device->dev, 0));
	if (TST_RET < 0) {
		SAFE_CLOSE(sfd);
		tst_brk(TBROK | TTERRNO,
			"fsconfig() failed to set source to %s", tst_device->dev);
	}

	TEST(fsconfig(sfd, FSCONFIG_CMD_CREATE, NULL, NULL, 0));
	if (TST_RET < 0) {
		SAFE_CLOSE(sfd);
		tst_brk(TBROK | TTERRNO, "fsconfig() created superblock");
	}

	TEST(fsmount(sfd, tc->flags, tc->mount_attrs));
	SAFE_CLOSE(sfd);

	if (TST_RET < 0) {
		tst_brk(TFAIL | TTERRNO,
			"fsmount() failed to create a mount object");
	}
	mfd = TST_RET;

	TEST(move_mount(mfd, "", AT_FDCWD, MNTPOINT, MOVE_MOUNT_F_EMPTY_PATH));
	SAFE_CLOSE(mfd);

	if (TST_RET < 0) {
		tst_brk(TBROK | TTERRNO,
			"move_mount() failed to attach to the mount point");
	}

	if (!tst_ismount(MNTPOINT))
		tst_res(TPASS, "%s: fsmount() passed", tc->name);

	SAFE_UMOUNT(MNTPOINT);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.format_device = 1,
	.all_filesystems = 1,
	.dev_fs_flags = TST_FS_SKIP_FUSE,
};
