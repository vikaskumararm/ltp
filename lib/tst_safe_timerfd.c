// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 */

#include "tst_safe_timerfd.h"
#include "lapi/timerfd.h"
#include "tst_clocks.h"
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

int safe_timerfd_create(const char *file, const int lineno,
				      int clockid, int flags)
{
	return TST_ASSERT_SYSCALL_FD_IMPL(timerfd_create(clockid, flags), file,
		lineno, "timerfd_create(%s)", tst_clock_name(clockid));
}

int safe_timerfd_gettime(const char *file, const int lineno,
				int fd, struct itimerspec *curr_value)
{
	return TST_ASSERT_SYSCALL_IMPL(timerfd_gettime(fd, curr_value), file,
		lineno, "timerfd_gettime()");
}

int safe_timerfd_settime(const char *file, const int lineno,
				int fd, int flags,
				const struct itimerspec *new_value,
				struct itimerspec *old_value)
{
	return TST_ASSERT_SYSCALL_IMPL(timerfd_settime(fd, flags, new_value,
		old_value), file, lineno, "timerfd_settime()");
}
