/*
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd
 *          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * Copyright (c) 2016 Linux Test Project
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */
#define _XOPEN_SOURCE 600
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

#include "mq.h"

#include "tst_test.h"
#include "tst_safe_posix_ipc.h"

#define MAX_MSGSIZE     8192
#define MSG_SIZE	16
#define USER_DATA       0x12345678
#define QUEUE_NAME	"/test_mqueue"

static char *str_debug;
static struct sigevent ev;

static volatile sig_atomic_t notified, cmp_ok;
static siginfo_t info;

struct test_case {
	void (*setup)(void);
	void (*cleanup)(void);
	int fd;
	int notify;
	int ttype;
	int ret;
	int err;
};

static void create_queue_notify_queue(void)
{
	create_queue();
	if (mq_notify(fd, &ev) == -1)
		tst_brk(TBROK | TERRNO, "mq_notify(%d, %p) failed", fd, &ev);
}

static struct test_case tcase[] = {
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.notify = SIGEV_NONE,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.notify = SIGEV_SIGNAL,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.notify = SIGEV_THREAD,
		.ret = 0,
		.err = 0,
	},
	{
		.fd = -1,
		.notify = SIGEV_NONE,
		.ret = -1,
		.err = EBADF,
	},
	{
		.fd = INT_MAX - 1,
		.notify = SIGEV_NONE,
		.ret = -1,
		.err = EBADF,
	},
	{
		.setup = open_fd,
		.notify = SIGEV_NONE,
		.ret = -1,
		.err = EBADF,
	},
	{
		.setup = create_queue_notify_queue,
		.cleanup = unlink_queue,
		.notify = SIGEV_NONE,
		.ret = -1,
		.err = EBUSY,
	},
};

static void setup(void)
{
	int i;

	for (i = 0; i < MSG_SIZE; i++)
		smsg[i] = i;
}
static void sigfunc(int signo LTP_ATTRIBUTE_UNUSED, siginfo_t *si,
	void *data LTP_ATTRIBUTE_UNUSED)
{
	if (str_debug)
		memcpy(&info, si, sizeof(info));

	cmp_ok = si->si_code == SI_MESGQ &&
	    si->si_signo == SIGUSR1 &&
	    si->si_value.sival_int == USER_DATA &&
	    si->si_pid == getpid() && si->si_uid == getuid();
	notified = 1;
}

static void tfunc(union sigval sv)
{
	cmp_ok = sv.sival_int == USER_DATA;
	notified = 1;
}

static void do_test(unsigned int i)
{
	int rc;
	struct sigaction sigact;
	struct timespec abs_timeout;
	struct test_case *tc = &tcase[i];

	notified = cmp_ok = 1;

	/* Don't timeout. */
	abs_timeout.tv_sec = 0;
	abs_timeout.tv_nsec = 0;

	/*
	 * When test ended with SIGTERM etc, mq descriptor is left remains.
	 * So we delete it first.
	 */
	mq_unlink(QUEUE_NAME);

	if (tc->fd)
		fd = tc->fd;

	if (tc->setup)
		tc->setup();

	ev.sigev_notify = tc->notify;

	switch (tc->notify) {
	case SIGEV_SIGNAL:
		notified = cmp_ok = 0;
		ev.sigev_signo = SIGUSR1;
		ev.sigev_value.sival_int = USER_DATA;

		memset(&sigact, 0, sizeof(sigact));
		sigact.sa_sigaction = sigfunc;
		sigact.sa_flags = SA_SIGINFO;
		rc = sigaction(SIGUSR1, &sigact, NULL);
		break;
	case SIGEV_THREAD:
		notified = cmp_ok = 0;
		ev.sigev_notify_function = tfunc;
		ev.sigev_notify_attributes = NULL;
		ev.sigev_value.sival_int = USER_DATA;
		break;
	}

	/* test */
	TEST(mq_notify(fd, &ev));
	if (TEST_RETURN >= 0) {
		rc = mq_timedsend(fd, smsg, MSG_SIZE, 0, &abs_timeout);
		if (rc < 0) {
			tst_res(TFAIL | TTERRNO, "mq_timedsend failed");
			goto CLEANUP;
		}

		while (!notified)
			usleep(10000);

		if (str_debug && tc->notify == SIGEV_SIGNAL) {
			tst_res(TINFO, "si_code  E:%d,\tR:%d",
				info.si_code, SI_MESGQ);
			tst_res(TINFO, "si_signo E:%d,\tR:%d",
				info.si_signo, SIGUSR1);
			tst_res(TINFO, "si_value E:0x%x,\tR:0x%x",
				info.si_value.sival_int, USER_DATA);
			tst_res(TINFO, "si_pid   E:%d,\tR:%d",
				info.si_pid, getpid());
			tst_res(TINFO, "si_uid   E:%d,\tR:%d",
				info.si_uid, getuid());
		}
	}

	if ((TEST_RETURN != 0 && TEST_ERRNO != tc->err) || !cmp_ok) {
		tst_res(TFAIL | TTERRNO, "mq_notify r/w check returned: %ld, "
			"expected: %d, expected errno: %s (%d)",
			TEST_RETURN, tc->ret, tst_strerrno(tc->err), tc->err);
	} else {
		tst_res(TPASS | TTERRNO, "mq_notify returned: %ld", TEST_RETURN);
	}

CLEANUP:
	if (fd >= 0) {
		close(fd);
		mq_unlink(QUEUE_NAME);
	}
}

static struct tst_option options[] = {
	{"d", &str_debug, "Print debug messages"},
	{NULL, NULL, NULL}
};

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.options = options,
	.setup = setup,
};
