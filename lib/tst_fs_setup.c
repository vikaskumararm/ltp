// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * DESCRIPTION
 *	A place for setup filesystem helpers.
 */

#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/vfs.h>
#include <sys/mount.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_fs.h"

void create_overlay_dirs(void)
{
	SAFE_MKDIR(OVL_LOWER, 0755);
	SAFE_MKDIR(OVL_UPPER, 0755);
	SAFE_MKDIR(OVL_WORK, 0755);
	SAFE_MKDIR(OVL_MNT, 0755);
}

int mount_overlay(const char *file, const int lineno, int safe)
{
	int ret = 0;
	char *cfgmsg = "overlayfs is not configured in this kernel.";

	ret = mount("overlay", OVL_MNT, "overlay", 0, "lowerdir="OVL_LOWER
		    ",upperdir="OVL_UPPER",workdir="OVL_WORK);
	if (ret < 0) {
		if (errno == ENODEV) {
			if (safe) {
				tst_brk(TCONF, cfgmsg);
			} else {
				tst_res(TINFO, cfgmsg);
				return 1;
			}
		} else {
			tst_brk(TBROK | TERRNO, "overlayfs mount failed");
		}
	}
	return 0;
}
