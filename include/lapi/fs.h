// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Referred from linux kernel include/uapi/linux/fs.h
 * Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email: code@zilogic.com
 */

#ifdef HAVE_LINUX_FS_H
# include <linux/fs.h>
#endif
# include <sys/user.h>
# include "lapi/abisize.h"

#ifndef LAPI_FS_H
#define LAPI_FS_H

#ifndef FS_IOC_GETFLAGS
#define	FS_IOC_GETFLAGS	_IOR('f', 1, long)
#endif

#ifndef FS_IOC_SETFLAGS
#define	FS_IOC_SETFLAGS	_IOW('f', 2, long)
#endif

#ifndef FS_COMPR_FL
#define	FS_COMPR_FL        0x00000004 /* Compress file */
#endif

#ifndef FS_IMMUTABLE_FL
#define FS_IMMUTABLE_FL	   0x00000010 /* Immutable file */
#endif

#ifndef FS_APPEND_FL
#define FS_APPEND_FL	   0x00000020 /* writes to file may only append */
#endif

#ifndef FS_NODUMP_FL
#define FS_NODUMP_FL	   0x00000040 /* do not dump file */
#endif

#define TST_GET_MAX_32BIT \
({ \
        long page_size = getpagesize(); \
        loff_t ret = ULONG_MAX; \
 \
        while (page_size >>= 1) \
                ret <<= 1; \
 \
        ret; \
})

/*
 * 64 bit macro taken from kernel include/linux/fs.h,
 * 32 bit macro replaced due missing PAGE_SHIFT on some libc.
 */
#ifdef TST_ABI64
 #define MAX_LFS_FILESIZE   ((loff_t)LLONG_MAX)
#else
 #define MAX_LFS_FILESIZE   TST_GET_MAX_32BIT
#endif

#endif
