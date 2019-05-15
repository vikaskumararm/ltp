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

int setup_overlay(int mountovl)
{
	int ret;

	/* Setup an overlay mount with lower dir and file */
	SAFE_MKDIR(OVL_LOWER, 0755);
	SAFE_MKDIR(OVL_UPPER, 0755);
	SAFE_MKDIR(OVL_WORK, 0755);
	SAFE_MKDIR(OVL_MNT, 0755);

	/* Only create dirs, do not mount */
	if (mountovl == 0)
		return 0;

	ret = mount("overlay", OVL_MNT, "overlay", 0, "lowerdir="OVL_LOWER
		    ",upperdir="OVL_UPPER",workdir="OVL_WORK);
	if (ret < 0) {
		if (errno == ENODEV) {
			tst_res(TINFO,
				"overlayfs is not configured in this kernel.");
			return 1;
		}
		tst_brk(TBROK | TERRNO, "overlayfs mount failed");
	}
	return 0;
}
