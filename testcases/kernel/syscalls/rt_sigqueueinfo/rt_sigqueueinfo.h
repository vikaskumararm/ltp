// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

#ifndef RT_SIGQUEUEINFO_H__
#define RT_SIGQUEUEINFO_H__

#define gettid() syscall(SYS_gettid)

#include "lapi/syscalls.h"

static int sys_rt_sigqueueinfo(pid_t tgid, int sig, siginfo_t *uinfo)
{
	return tst_syscall(__NR_rt_sigqueueinfo, tgid, sig, uinfo);
}

#endif /* RT_SIGQUEUEINFO_H__ */
