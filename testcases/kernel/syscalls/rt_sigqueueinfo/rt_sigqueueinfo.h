// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

#ifdef HAVE_STRUCT_SIGACTION_SA_SIGACTION

#ifndef __RT_SIGQUEUEINFO_H__
#define __RT_SIGQUEUEINFO_H__

#define gettid() syscall(SYS_gettid)

#include "lapi/syscalls.h"

static int sys_rt_sigqueueinfo(pid_t tgid, int sig, siginfo_t *uinfo)
{
	return tst_syscall(__NR_rt_sigqueueinfo, tgid, sig, uinfo);
}

#endif /* __RT_SIGQUEUEINFO_H__ */
#endif /* HAVE_STRUCT_SIGACTION_SA_SIGACTION */
