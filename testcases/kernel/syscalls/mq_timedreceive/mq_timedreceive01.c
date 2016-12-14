/*
 * Copyright (c) Crackerjack Project., 2007-2008 ,Hitachi, Ltd
 *          Author(s): Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 *		       Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 *		       Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 * Copyright (c) 2016-2017 Linux Test Project
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
static int fd, fd_root;

struct test_case {
	int len;
	unsigned prio;
	struct timespec rq;
	int fd;
	int invalid_msg;
	int send;
	int ret;
	int err;
	void (*setup)(void);
	void (*cleanup)(void);
};

#define MAX_MSGSIZE     8192

#define QUEUE_NAME	"/test_mqueue"

static void create_queue(void);
static void create_queue_nonblock(void);
static void create_queue_sig(void);
static void open_fd(void);
static void unlink_queue(void);

static struct test_case tcase[] = {
	{
		.setup = create_queue,
		.send = 1,
		.len = 0,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.send = 1,
		.len = 1,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.send = 1,
		.len = MAX_MSGSIZE,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.send = 1,
		.len = 1,
		.prio = 32767,	/* max priority */
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.invalid_msg = 1,
		.send = 1,
		.len = 0,
		.ret = -1,
		.err = EMSGSIZE,
	},
	{
		.len = 0,
		.fd = -1,
		.ret = -1,
		.err = EBADF,
	},
	{
		.len = 0,
		.fd = INT_MAX - 1,
		.ret = -1,
		.err = EBADF,
	},
	{
		.len = 0,
		.ret = -1,
		.err = EBADF,
		.setup = open_fd,
	},
	{
		.len = 16,
		.ret = -1,
		.err = EAGAIN,
		.setup = create_queue_nonblock,
		.cleanup = unlink_queue,
	},
	{
		.len = 16,
		.rq = (struct timespec) {.tv_sec = -1, .tv_nsec = 0},
		.ret = -1,
		.err = EINVAL,
		.setup = create_queue,
		.cleanup = unlink_queue,
	},
	{
		.len = 16,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = -1},
		.ret = -1,
		.err = EINVAL,
		.setup = create_queue,
		.cleanup = unlink_queue,
	},
	{
		.len = 16,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = 1000000000},
		.ret = -1,
		.err = EINVAL,
		.setup = create_queue,
		.cleanup = unlink_queue,
	},
	{
		.len = 16,
		.rq = (struct timespec) {.tv_sec = 0, .tv_nsec = 999999999},
		.ret = -1,
		.err = ETIMEDOUT,
		.setup = create_queue,
		.cleanup = unlink_queue,
	},
	{
		.len = 16,
		.rq = (struct timespec) {.tv_sec = 3, .tv_nsec = 0},
		.ret = -1,
		.err = EINTR,
		.setup = create_queue_sig,
		.cleanup = unlink_queue,
	},
};

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

static void setup(void)
{
	act.sa_handler = sighandler;
	sigaction(SIGINT, &act, NULL);

	fd_root = SAFE_OPEN("/", O_RDONLY);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static void create_queue(void)
{
	fd = mq_open(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRWXU, NULL);
	if (fd == -1)
		tst_brk(TBROK | TERRNO, "mq_open(" QUEUE_NAME ") failed");
}

static void create_queue_nonblock(void)
{
	fd = mq_open(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, S_IRWXU,
		     NULL);
	if (fd == -1)
		tst_brk(TBROK | TERRNO, "mq_open(" QUEUE_NAME ") failed");
}

static void create_queue_sig(void)
{
	create_queue();
	pid = create_sig_proc(SIGINT, 40, 200000);
}

static void open_fd(void)
{
	fd = fd_root;
}

static void send_msg(int fd, int len, int prio)
{
	char smsg[MAX_MSGSIZE];
	int i;

	for (i = 0; i < len; i++)
		smsg[i] = i;

	if (mq_timedsend(fd, smsg, len, prio,
		&((struct timespec){0})) < 0)
		tst_brk(TBROK | TERRNO, "mq_timedsend failed");
}

static void unlink_queue(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	mq_unlink(QUEUE_NAME);
}

static void do_test(unsigned int i)
{
	struct test_case *tc = &tcase[i];

	char rmsg[MAX_MSGSIZE];
	unsigned prio;
	size_t msg_len;

	fd = -1;
	pid = 0;

	/*
	 * When test ended with SIGTERM etc, mq descriptor is left remains.
	 * So we delete it first.
	 */
	mq_unlink(QUEUE_NAME);

	if (tc->fd)
		fd = tc->fd;

	if (tc->setup)
		tc->setup();

	if (tc->send)
		send_msg(fd, tc->len, tc->prio);

	msg_len = MAX_MSGSIZE;
	if (tc->invalid_msg)
		msg_len -= 1;

	if (tc->rq.tv_sec >= 0 || tc->rq.tv_nsec != 0)
		tc->rq.tv_sec += time(NULL);

	/* test */
	TEST(mq_timedreceive(fd, rmsg, msg_len, &prio, &tc->rq));

	/* cleanup */
	if (tc->cleanup)
		tc->cleanup();

	if (pid) {
		SAFE_KILL(pid, SIGTERM);
		SAFE_WAIT(NULL);
	}

	/* result */
	if (TEST_RETURN != -1) {
		if (TEST_RETURN != tc->len) {
			tst_res(TFAIL | TTERRNO, "mq_timedreceive wrong msg_len returned %ld, expected %d",
				TEST_RETURN, tc->len);
			return;
		}

		if (tc->prio != prio) {
			tst_res(TFAIL | TTERRNO, "mq_timedreceive wrong prio returned %d, expected %d",
				prio, tc->prio);
			return;
		}
	}

	if (TEST_ERRNO != tc->err || (tc->ret < 0 && TEST_RETURN != tc->ret)
		|| (tc->ret >= 0 && TEST_RETURN < 0))
		tst_res(TFAIL | TTERRNO, "%d returned: %ld, expected: %d, expected errno: %s (%d)",
				i, TEST_RETURN, tc->ret, tst_strerrno(tc->err), tc->err);
	else
		tst_res(TPASS | TTERRNO, "%d returned: %ld", i, TEST_RETURN);
}

static struct tst_test test = {
	.tid = "mq_timedreceive01",
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
