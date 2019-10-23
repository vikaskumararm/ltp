// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 *
 * Test Name: quotactl05
 *
 * Description:
 * This testcase checks basic flags of quotactl(2) for project on an XFS file
 * system:
 * 1) quotactl(2) succeeds to turn off xfs quota and get xfs quota off status
 *    for project
 * 2) quotactl(2) succeeds to turn on xfs quota and get xfs quota on status
 *    for project.
 * 3) quotactl(2) succeeds to set and use Q_XGETQUOTA to get xfs disk quota
 *    limits for project.
 * 4) quotactl(2) succeeds to set and use Q_XGETNEXTQUOTA to get xfs disk
 *    quota limits Cgreater than or equal to ID for project.
 * 5) quotactl(2) succeeds to turn off xfs quota and get xfs quota off statv
 *    for project.
 * 6) quotactl(2) succeeds to turn on xfs quota and get xfs quota on statv
 *    for project.
 * 7)quotactl(2) succeeds to turn off xfs_quota and use Q_XQUOTARM to free
 *    disk space taken by disk quotas.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/quota.h>
#include "config.h"
#include "tst_test.h"
#include "lapi/quotactl.h"

#if defined(HAVE_XFS_XQM_H)
#include <xfs/xqm.h>
static void check_qoff(int, char *, int);
static void check_qon(int, char *, int);

#if defined(HAVE_STRUCT_FS_QUOTA_STATV)
static void check_qoffv(int, char *, int);
static void check_qonv(int, char *, int);
#endif

static void check_qlim(int, char *);
static void check_qrm(int, char *);

static uint32_t test_id;
static struct fs_disk_quota set_dquota = {
	.d_rtb_softlimit = 1000,
	.d_fieldmask = FS_DQ_RTBSOFT
};
static uint32_t qflag = XFS_QUOTA_PDQ_ENFD;
static uint32_t qflag_acct = XFS_QUOTA_PDQ_ACCT | XFS_QUOTA_UDQ_ACCT | XFS_QUOTA_GDQ_ACCT;
static const char mntpoint[] = "mnt_point";

static struct t_case {
	int cmd;
	void *addr;
	void (*func_check)();
	int check_subcmd;
	int flag;
	char *des;
} tcases[] = {
	{QCMD(Q_XQUOTAOFF, PRJQUOTA), &qflag, check_qoff, Q_XGETQSTAT, 1,
	"turn off xfs quota and get xfs quota off status for project"},

	{QCMD(Q_XQUOTAON, PRJQUOTA), &qflag, check_qon, Q_XGETQSTAT, 1,
	"turn on xfs quota and get xfs quota on status for project"},

	{QCMD(Q_XSETQLIM, PRJQUOTA), &set_dquota, check_qlim, Q_XGETQUOTA, 0,
	"Q_XGETQUOTA for project"},

	{QCMD(Q_XSETQLIM, PRJQUOTA), &set_dquota, check_qlim, Q_XGETNEXTQUOTA, 0,
	"Q_XGETNEXTQUOTA for project"},

#if defined(HAVE_STRUCT_FS_QUOTA_STATV)
	{QCMD(Q_XQUOTAOFF, PRJQUOTA), &qflag, check_qoffv, Q_XGETQSTATV, 1,
	"turn off xfs quota and get xfs quota off statv for project"},

	{QCMD(Q_XQUOTAON, PRJQUOTA), &qflag, check_qonv, Q_XGETQSTATV, 1,
	"turn on xfs quota and get xfs quota on statv for project"},
#endif

	{QCMD(Q_XQUOTAOFF, PRJQUOTA), &qflag_acct, check_qrm, Q_XQUOTARM, 1,
	"turn off xfs quota and free disk spaces taken by disk quotas"},
};

static void check_qoff(int subcmd, char *desp, int flag)
{
	int res;
	struct fs_quota_stat res_qstat;

	res = quotactl(QCMD(subcmd, PRJQUOTA), tst_device->dev,
			test_id, (void *) &res_qstat);
	if (res == -1) {
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to get xfs quota off status");
		return;
	}

	if (res_qstat.qs_flags & flag) {
		tst_res(TFAIL, "xfs quota enforcement was on unexpectedly");
		return;
	}

	tst_res(TPASS, "quoactl() succeeded to %s", desp);
}

#if defined(HAVE_STRUCT_FS_QUOTA_STATV)
static void check_qoffv(int subcmd, char *desp, int flag)
{
	int res;
	struct fs_quota_statv res_qstatv = {
		.qs_version = FS_QSTATV_VERSION1,
	};

	res = quotactl(QCMD(subcmd, PRJQUOTA), tst_device->dev,
			test_id, (void *) &res_qstatv);
	if (res == -1) {
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to get xfs quota off stav");
		return;
	}

	if (res_qstatv.qs_flags & flag) {
		tst_res(TFAIL, "xfs quota enforcement was on unexpectedly");
		return;
	}

	tst_res(TPASS, "quoactl() succeeded to %s", desp);
}
#endif

static void check_qon(int subcmd, char *desp, int flag)
{
	int res;
	struct fs_quota_stat res_qstat;

	res = quotactl(QCMD(subcmd, PRJQUOTA), tst_device->dev,
		test_id, (void *) &res_qstat);
	if (res == -1) {
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to get xfs quota on status");
		return;
	}

	if (!(res_qstat.qs_flags & flag)) {
		tst_res(TFAIL, "xfs quota enforcement was off unexpectedly");
		return;
	}

	tst_res(TPASS, "quoactl() succeeded to %s", desp);
}

#if defined(HAVE_STRUCT_FS_QUOTA_STATV)
static void check_qonv(int subcmd, char *desp, int flag)
{
	int res;
	struct fs_quota_statv res_qstatv = {
		.qs_version = FS_QSTATV_VERSION1
	};

	res = quotactl(QCMD(subcmd, PRJQUOTA), tst_device->dev,
		test_id, (void *) &res_qstatv);
	if (res == -1) {
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to get xfs quota on statv");
		return;
	}

	if (!(res_qstatv.qs_flags & flag)) {
		tst_res(TFAIL, "xfs quota enforcement was off unexpectedly");
		return;
	}

	tst_res(TPASS, "quoactl() succeeded to %s", desp);
}
#endif

static void check_qlim(int subcmd, char *desp)
{
	int res;
	static struct fs_disk_quota res_dquota;

	res_dquota.d_rtb_softlimit = 0;

	res = quotactl(QCMD(subcmd, PRJQUOTA), tst_device->dev,
			test_id, (void *) &res_dquota);
	if (res == -1) {
		if (errno == EINVAL) {
			tst_brk(TCONF | TERRNO,
				"%s wasn't supported in quotactl()", desp);
		}
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to get xfs disk quota limits");
		return;
	}

	if (res_dquota.d_id != test_id) {
		tst_res(TFAIL, "quotactl() got unexpected user id %u,"
			" expected %u", res_dquota.d_id, test_id);
		return;
	}

	if (res_dquota.d_rtb_hardlimit != set_dquota.d_rtb_hardlimit) {
		tst_res(TFAIL, "quotactl() got unexpected rtb soft limit %llu,"
			" expected %llu", res_dquota.d_rtb_hardlimit,
			set_dquota.d_rtb_hardlimit);
		return;
	}

	tst_res(TPASS, "quoactl() succeeded to set and use %s to get xfs disk "
		"quota limits", desp);
}

static void check_qrm(int subcmd, char *desp)
{
	int res;

	res = quotactl(QCMD(subcmd, PRJQUOTA), tst_device->dev,
			test_id, (void *) &qflag_acct);
	if (res == -1) {
		if (errno == EINVAL) {
			tst_brk(TCONF | TERRNO,
				"Q_XQUOTARM wasn't supported in quotactl()");
		}
		tst_res(TFAIL | TERRNO,
			"quotactl() failed to free disk spaces taken by disk quotas");
		return;
	}

	tst_res(TPASS, "quoactl() succeeded to %s", desp);
}

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

static const char *kconfigs[] = {
	"CONFIG_XFS_QUOTA",
	NULL
};

static struct tst_test test = {
	.needs_tmpdir = 1,
	.needs_root = 1,
	.needs_kconfigs = kconfigs,
	.test = verify_quota,
	.tcnt = ARRAY_SIZE(tcases),
	.mount_device = 1,
	.dev_fs_type = "xfs",
	.mntpoint = mntpoint,
	.mnt_data = "prjquota",
	.setup = setup,
};
#else
	TST_TEST_TCONF("This system didn't have <xfs/xqm.h>");
#endif
