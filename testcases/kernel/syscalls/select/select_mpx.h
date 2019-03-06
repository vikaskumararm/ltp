/*
 * Copyright (C) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef SELECT_MPX
#define SELECT_MPX

#include "lapi/syscalls.h"

static int sys_mpx = -1;

struct compat_sel_arg_struct {
	long _n;
	long _inp;
	long _outp;
	long _exp;
	long _tvp;
};

static int do_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	switch (sys_mpx) {
	case 0:
		return select(nfds, readfds, writefds, exceptfds, timeout);
	break;
	case 1:
		return tst_syscall(__NR__newselect, nfds, readfds, writefds, exceptfds, timeout);
	break;
	case 2: {
		struct compat_sel_arg_struct arg = {
			._n = (long)nfds,
			._inp = (long)readfds,
			._outp = (long)writefds,
			._exp = (long)exceptfds,
			._tvp = (long)timeout,
		};

		return tst_syscall(__NR_select, &arg);
	}
	case 3: {
		int ret;
		struct timespec ts = {
			.tv_sec = timeout->tv_sec,
			.tv_nsec = timeout->tv_usec * 1000,
		};
		ret = tst_syscall(__NR_pselect6, nfds, readfds, writefds, exceptfds, &ts, NULL);
		timeout->tv_sec = ts.tv_sec;
		timeout->tv_usec = ts.tv_nsec / 1000;
		return ret;
	}
	}

	return -1;
}

static int select_mpx(void)
{
	switch (++sys_mpx) {
	case 0:
		tst_res(TINFO, "Testing glibc select()");
	break;
	case 1:
		tst_res(TINFO, "Testing SYS__newselect syscall");
	break;
	case 2:
		tst_res(TINFO, "Testing SYS_select syscall");
	break;
	case 3:
		tst_res(TINFO, "Testing SYS_pselect6 syscall");
	break;
	case 4:
		sys_mpx = -1;
		return 0;
	}

	return 1;
}

#endif /* SELECT_MPX */
