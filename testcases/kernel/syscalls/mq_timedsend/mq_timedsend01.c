/*
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd
 *          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * Copyright (c) 2016 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <errno.h>
#include <limits.h>
#include <mqueue.h>

#include "tst_sig_proc.h"
#include "tst_test.h"

static struct sigaction act;
static pid_t pid;
static int fd;

enum test_type {
	NORMAL,
	FD_NONE,
	FD_NOT_EXIST,
	FD_FILE,
	FULL_QUEUE,
	SEND_SIGINT,
};

struct test_case {
	int ttype;
	const char *desc;
	int non_block;
	int len;
	unsigned prio;
	struct timespec rq;
	int ret;
	int err;
};

#define MAX_MSG	 10
#define MAX_MSGSIZE     8192
#define QUEUE_NAME	"/test_mqueue"

#define TYPE_NAME(x) .ttype = x, .desc = #x
static struct test_case tcase[] = {
	{
		TYPE_NAME(NORMAL),
		.len = 0,
		.ret = 0,
		.err = 0,
	},
	{
		TYPE_NAME(NORMAL),
		.len = 1,
		.ret = 0,
		.err = 0,
	},
	{
		TYPE_NAME(NORMAL),
		.len = MAX_MSGSIZE,
		.ret = 0,
		.err = 0,
	},
	{
		TYPE_NAME(NORMAL),
		.len = 1,
		.prio = 32767,	/* max priority */
		.ret = 0,
		.err = 0,
	},
	{
		TYPE_NAME(NORMAL),
		.len = MAX_MSGSIZE + 1,
		.ret = -1,
		.err = EMSGSIZE,
	},
	{
		TYPE_NAME(FD_NONE),
		.len = 0,
		.ret = -1,
		.err = EBADF,
	},
	{
		TYPE_NAME(FD_NOT_EXIST),
		.len = 0,
		.ret = -1,
		.err = EBADF,
	},
	{
		TYPE_NAME(FD_FILE),
		.len = 0,
		.ret = -1,
		.err = EBADF,
	},
	{
		TYPE_NAME(FULL_QUEUE),
		.non_block = 1,
		.len = 16,
		.ret = -1,
		.err = EAGAIN,
	},
	{
		TYPE_NAME(NORMAL),
		.len = 1,
		.prio = 32768,	/* max priority + 1 */
		.ret = -1,
		.err = EINVAL,
	},
	{
		TYPE_NAME(FULL_QUEUE),
		.len = 16,
		.rq = (struct timespec) {.tv_sec = -1, .tv_nsec = 0},
		.ret = -1,
		.err = EINVAL,
	},
	{
		TYPE_NAME(FULL_QUEUE),
		.len = 16,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = -1},
		.ret = -1,
		.err = EINVAL,
	},
	{
		TYPE_NAME(FULL_QUEUE),
		.len = 16,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = 1000000000},
		.ret = -1,
		.err = EINVAL,
	},
	{
		TYPE_NAME(FULL_QUEUE),
		.len = 16,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = 999999999},
		.ret = -1,
		.err = ETIMEDOUT,
	},
	{
		TYPE_NAME(SEND_SIGINT),
		.len = 16,
		.ret = -1,
		.rq = (struct timespec) {.tv_sec = 3, .tv_nsec = 0},
		.err = EINTR,
	},
};

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

static void setup(void)
{
	SAFE_SIGNAL(SIGINT, sighandler);
	act.sa_handler = sighandler;
	sigaction(SIGINT, &act, NULL);
}

static void cleanup(void)
{
	if (fd > 0)
		close(fd);
}

static void cleanup_n(unsigned int i)
{
	struct test_case *tc = &tcase[i];

	if (fd > 0 && tc->ttype != FD_NOT_EXIST)
		SAFE_CLOSE(fd);

	if (pid) {
		SAFE_KILL(pid, SIGTERM);
		SAFE_WAIT(NULL);
	}

	mq_unlink(QUEUE_NAME);
}

static void do_test(unsigned int i)
{
	struct test_case *tc = &tcase[i];

	int oflag;
	int j;
	char smsg[MAX_MSGSIZE], rmsg[MAX_MSGSIZE];
	unsigned prio;

	fd = -1;
	pid = 0;

	tst_res(TINFO, "case %s", tc->desc);

	/*
	 * When test ended with SIGTERM etc, mq descriptor is left remains.
	 * So we delete it first.
	 */
	TEST(mq_unlink(QUEUE_NAME));

	switch (tc->ttype) {
	case FD_NONE:
		break;
	case FD_NOT_EXIST:
		fd = INT_MAX - 1;
		break;
	case FD_FILE:
		fd = open("/", O_RDONLY);
		if (fd < 0) {
			tst_res(TBROK | TERRNO, "can't open \"/\".");
			cleanup_n(i);
			return;
		}
		break;
	default:
		oflag = O_CREAT | O_EXCL | O_RDWR;
		if (tc->non_block)
			oflag |= O_NONBLOCK;

		fd = mq_open(QUEUE_NAME, oflag, S_IRWXU, NULL);
		if (fd < 0) {
			tst_res(TBROK | TERRNO, "mq_open failed");
			cleanup_n(i);
			return;
		}

		if (tc->ttype == FULL_QUEUE || tc->ttype == SEND_SIGINT) {
			for (j = 0; j < MAX_MSG; j++) {
				if (mq_timedsend(fd, smsg, tc->len, 0,
							&((struct timespec){0})) < 0) {
					tst_res(TBROK | TERRNO, "mq_timedsend failed");
					cleanup_n(i);
					return;
				}
			}
			if (tc->ttype == SEND_SIGINT) {
				pid = create_sig_proc(SIGINT, 4, 200000);
			}
		}
	}

	for (j = 0; j < tc->len && (unsigned)j < sizeof(smsg); j++)
		smsg[j] = j;

	if (tc->rq.tv_sec >= 0 || tc->rq.tv_nsec != 0)
		tc->rq.tv_sec += time(NULL);

	/* send */
	TEST(mq_timedsend(fd, smsg, tc->len, tc->prio, &tc->rq));

	if (TEST_RETURN != -1) {
		/* receive */
		tc->rq.tv_sec = 0;
		tc->rq.tv_nsec = 0;
		TEST(mq_timedreceive(fd, rmsg, MAX_MSGSIZE, &prio, &tc->rq));

		cleanup_n(i);

		if (TEST_RETURN < 0) {
			tst_res(TFAIL | TTERRNO, "mq_timedreceive failed");
			return;
		}

		if (TEST_RETURN != tc->len) {
			tst_res(TFAIL | TTERRNO, "mq_timedreceive msg_len returned %ld, expected %d",
				TEST_RETURN, tc->len);
			return;
		}

		if (tc->prio != prio) {
			tst_res(TFAIL | TTERRNO, "mq_timedreceive msg_prio returned %d, expected %d",
				prio, tc->prio);
			return;
		}

		for (j = 0; j < tc->len; j++) {
			if (rmsg[j] != smsg[j]) {
				tst_res(TFAIL | TTERRNO, "mq_timedreceive wrong returned message index: %d",
					j);
				return;
			}
		}
	} else
		cleanup_n(i);

	if (TEST_ERRNO != tc->err || (tc->ret < 0 && TEST_RETURN != tc->ret) ||
		(tc->ret >= 0 && TEST_RETURN < 0)) {
		tst_res(TFAIL | TTERRNO, "%s returned: %ld, "
			"expected: %d, expected errno: %s (%d)", tc->desc,
			TEST_RETURN, tc->ret, tst_strerrno(tc->err), tc->err);
	} else {
		tst_res(TPASS | TTERRNO, "%s returned: %ld", tc->desc,
			TEST_RETURN);
	}
}

static struct tst_test test = {
	.tid = "mq_timedsend",
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
