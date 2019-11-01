// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2019 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Author: Yang Xu <xuyang2018.jy@cn.jujitsu.com>
 */

#ifndef LAPI_QUOTACTL_H__
# define LAPI_QUOTACTL_H__

#ifdef HAVE_STRUCT_IF_NEXTDQBLK
# include <linux/quota.h>
#endif
#include <sys/quota.h>

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
