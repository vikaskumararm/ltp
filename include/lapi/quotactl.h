// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2019 FUJITSU LIMITED. All rights reserved.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 */

#ifndef LAPI_QUOTACTL_H__
# define LAPI_QUOTACTL_H__

#include <linux/quota.h>

# ifndef PRJQUOTA
#  define PRJQUOTA 2
# endif

# ifndef Q_XQUOTARM
#  define Q_XQUOTARM XQM_CMD(6)
# endif

# ifndef Q_XGETQSTATV
#  define Q_XGETQSTATV XQM_CMD(8)
# endif

# ifndef Q_XGETNEXTQUOTA
#  define Q_XGETNEXTQUOTA XQM_CMD(9)
# endif

# ifndef Q_GETNEXTQUOTA
#  define Q_GETNEXTQUOTA 0x800009 /* get disk limits and usage >= ID */
# endif

#endif /* LAPI_QUOTACTL_H__ */
