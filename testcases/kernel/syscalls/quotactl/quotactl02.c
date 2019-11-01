// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013-2019 FUJITSU LIMITED. All rights reserved
 * Author: DAN LI <li.dan@cn.fujitsu.com>
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*
 * Test Name: quotactl02
 *
 * Description:
 * This testcase checks basic flags of quotactl(2) for an XFS file system:
 * 1) quotactl(2) succeeds to turn off xfs quota and get xfs quota off status
 *    for user.
 * 2) quotactl(2) succeeds to turn on xfs quota and get xfs quota on status
 *    for usr.
 * 3) quotactl(2) succeeds to set and use Q_XGETQUOTA to get xfs disk quota
 *    limits for user.
 * 4) quotactl(2) succeeds to set and use Q_XGETNEXTQUOTA to get xfs disk
 *    quota limits greater than or equal to ID for user.
 * 5) quotactl(2) succeeds to turn off xfs quota and get xfs quota off statv
 *    for user.
 * 6) quotactl(2) succeeds to turn on xfs quota and get xfs quota on statv
 *    for user.
 * 7) quotactl(2) succeeds to turn off xfs quota and get xfs quota off status
 *    for group.
 * 8) quotactl(2) succeeds to turn on xfs quota and get xfs quota on status
 *    for group.
 * 9) quotactl(2) succeeds to set and use Q_XGETQUOTA to get xfs disk quota
 *    limits for group.
 * 10)quotactl(2) succeeds to set and use Q_XGETNEXTQUOTA to get xfs disk
 *    quota limits for group.
 * 11)quotactl(2) succeeds to turn off xfs quota and get xfs quota off statv
 *    for group.
 * 12)quotactl(2) succeeds to turn on xfs quota and get xfs quota on statv
 *    for group.
 */

#include "quotactl02.h"

#ifdef HAVE_XFS_XQM_H
static uint32_t qflagu = XFS_QUOTA_UDQ_ENFD;
static uint32_t qflagg = XFS_QUOTA_GDQ_ENFD;

static struct t_case {
	int cmd;
	void *addr;
	void (*func_check)();
	int check_subcmd;
	int flag;
	char *des;
} tcases[] = {
	{QCMD(Q_XQUOTAOFF, USRQUOTA), &qflagu, check_qoff,
	QCMD(Q_XGETQSTAT, USRQUOTA), 1,
	"turn off xfs quota and get xfs quota off status for user"},

	{QCMD(Q_XQUOTAON, USRQUOTA), &qflagu, check_qon,
	QCMD(Q_XGETQSTAT, USRQUOTA), 1,
	"turn on xfs quota and get xfs quota on status for user"},

	{QCMD(Q_XSETQLIM, USRQUOTA), &set_dquota, check_qlim,
	QCMD(Q_XGETQUOTA, USRQUOTA), 0,
	"Q_XGETQUOTA for user"},

	{QCMD(Q_XSETQLIM, USRQUOTA), &set_dquota, check_qlim,
	QCMD(Q_XGETNEXTQUOTA, USRQUOTA), 0,
	"Q_XGETNEXTQUOTA for user"},

#if defined(HAVE_STRUCT_FS_QUOTA_STATV)
	{QCMD(Q_XQUOTAOFF, USRQUOTA), &qflagu, check_qoffv,
	QCMD(Q_XGETQSTATV, USRQUOTA), 1,
	"turn off xfs quota and get xfs quota off statv for user"},

	{QCMD(Q_XQUOTAON, USRQUOTA), &qflagu, check_qonv,
	QCMD(Q_XGETQSTATV, USRQUOTA), 1,
	"turn on xfs quota and get xfs quota on statv for user"},
#endif

	{QCMD(Q_XQUOTAOFF, GRPQUOTA), &qflagg, check_qoff,
	QCMD(Q_XGETQSTAT, GRPQUOTA), 1,
	"turn off xfs quota and get xfs quota off status for group"},

	{QCMD(Q_XQUOTAON, GRPQUOTA), &qflagg, check_qon,
	QCMD(Q_XGETQSTAT, GRPQUOTA), 1,
	"turn on xfs quota and get xfs quota on status for group"},

	{QCMD(Q_XSETQLIM, GRPQUOTA), &set_dquota, check_qlim,
	QCMD(Q_XGETQUOTA, GRPQUOTA), 0,
	"Q_XGETQUOTA for group"},

	{QCMD(Q_XSETQLIM, GRPQUOTA), &set_dquota, check_qlim,
	QCMD(Q_XGETNEXTQUOTA, GRPQUOTA), 0,
	"Q_XGETNEXTQUOTA for group"},

#if defined(HAVE_STRUCT_FS_QUOTA_STATV)
	{QCMD(Q_XQUOTAOFF, GRPQUOTA), &qflagg, check_qoffv,
	QCMD(Q_XGETQSTATV, GRPQUOTA), 1,
	"turn off xfs quota and get xfs quota off statv for group"},

	{QCMD(Q_XQUOTAON, GRPQUOTA), &qflagg, check_qonv,
	QCMD(Q_XGETQSTATV, GRPQUOTA), 1,
	"turn on xfs quota and get xfs quota on statv for group"},
#endif
};

static void setup(void)
{
	test_id = geteuid();
}

static void verify_quota(unsigned int n)
{
	struct t_case *tc = &tcases[n];

	TEST(quotactl(tc->cmd, tst_device->dev, test_id, tc->addr));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "quotactl() failed to %s", tc->des);
		return;
	}

	if (tc->flag)
		tc->func_check(tc->check_subcmd, tc->des, *(int *)(tc->addr));
	else
		tc->func_check(tc->check_subcmd, tc->des);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.needs_kconfigs = kconfigs,
	.test = verify_quota,
	.tcnt = ARRAY_SIZE(tcases),
	.mount_device = 1,
	.dev_fs_type = "xfs",
	.mntpoint = mntpoint,
	.mnt_data = "usrquota,grpquota",
	.setup = setup,
};
#else
	TST_TEST_TCONF("System doesn't have <xfs/xqm.h>");
#endif
