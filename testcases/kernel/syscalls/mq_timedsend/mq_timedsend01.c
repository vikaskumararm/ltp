/*
 * Copyright (c) Crackerjack Project., 2007-2008, Hitachi, Ltd
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 *
 * Author(s):
 * Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 * Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 * Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <limits.h>

#include "mq.h"
#include "tst_test.h"

static struct sigaction act;
static pid_t pid;
static int fd;
char smsg[MAX_MSGSIZE];

struct test_case {
	void (*setup)(void);
	void (*cleanup)(void);
	int fd;
	unsigned int len;
	unsigned int prio;
	struct timespec *rq;
	int send;
	int ret;
	int err;
};

static struct test_case tcase[] = {
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.len = 0,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.len = 1,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.len = MAX_MSGSIZE,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.len = 1,
		.prio = MQ_PRIO_MAX - 1,
		.ret = 0,
		.err = 0,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.len = MAX_MSGSIZE + 1,
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
		.setup = open_fd,
		.len = 0,
		.ret = -1,
		.err = EBADF,
	},
	{
		.setup = create_queue_nonblock,
		.cleanup = unlink_queue,
		.len = 16,
		.send = 1,
		.ret = -1,
		.err = EAGAIN,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.len = 1,
		.prio = MQ_PRIO_MAX,
		.ret = -1,
		.err = EINVAL,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.len = 16,
		.rq = &(struct timespec) {.tv_sec = -1, .tv_nsec = 0},
		.send = 1,
		.ret = -1,
		.err = EINVAL,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.len = 16,
		.rq = &(struct timespec) {.tv_sec = 0, .tv_nsec = -1},
		.send = 1,
		.ret = -1,
		.err = EINVAL,
	},
	{
		.setup = create_queue,
		.cleanup = unlink_queue,
		.len = 16,
		.rq = &(struct timespec) {.tv_sec = 0, .tv_nsec = 1000000000},
		.send = 1,
		.ret = -1,
		.err = EINVAL,
	},
	{
		.setup = create_queue_timeout,
		.cleanup = unlink_queue,
		.len = 16,
		.rq = &timeout_ts,
		.send = 1,
		.ret = -1,
		.err = ETIMEDOUT,
	},
	{
		.setup = create_queue_sig,
		.cleanup = unlink_queue_sig,
		.len = 16,
		.ret = -1,
		.send = 1,
		.rq = &eintr_ts,
		.err = EINTR,
	},
};

static void sighandler(int sig LTP_ATTRIBUTE_UNUSED)
{
}

static void setup(void)
{
	int i;

	act.sa_handler = sighandler;
	sigaction(SIGINT, &act, NULL);

	fd_root = SAFE_OPEN("/", O_RDONLY);

	for (i = 0; i < MAX_MSGSIZE; i++)
		smsg[i] = i;
}

static void cleanup(void)
{
	if (fd > 0)
		close(fd);

	if (fd_root > 0)
		SAFE_CLOSE(fd_root);
}

static void do_test(unsigned int i)
{
	const struct test_case *tc = &tcase[i];
	unsigned int j;
	unsigned int prio;
	size_t len = MAX_MSGSIZE;
	char rmsg[len];

	/*
	 * When test ended with SIGTERM etc, mq descriptor is left remains.
	 * So we delete it first.
	 */
	mq_unlink(QUEUE_NAME);

	if (tc->fd)
		fd = tc->fd;

	if (tc->setup)
		tc->setup();

	if (tc->send) {
		for (j = 0; j < MAX_MSG; j++)
			send_msg(fd, tc->len, tc->prio);
	}

	TEST(mq_timedsend(fd, smsg, tc->len, tc->prio, tc->rq));

	if (TEST_RETURN < 0) {
		if (tc->err != TEST_ERRNO) {
			tst_res(TFAIL | TTERRNO,
				"mq_timedsend failed unexpectedly, expected %s",
				tst_strerrno(tc->err));
		} else {
			tst_res(TPASS | TTERRNO, "mq_timedreceive failed expectedly");
		}

		if (tc->cleanup)
			tc->cleanup();
		return;
	}

	TEST(mq_timedreceive(fd, rmsg, len, &prio, tc->rq));

	if (tc->cleanup)
		tc->cleanup();

	if (TEST_RETURN < 0) {
		if (tc->err != TEST_ERRNO) {
			tst_res(TFAIL | TTERRNO,
				"mq_timedreceive failed unexpectedly, expected %s",
				tst_strerrno(tc->err));
			return;
		}
	}

	if (tc->len != TEST_RETURN) {
		tst_res(TFAIL | TTERRNO, "mq_timedreceive wrong len returned %ld, expected %d",
			TEST_RETURN, tc->len);
		return;
	}

	if (tc->prio != prio) {
		tst_res(TFAIL | TTERRNO, "mq_timedreceive wrong prio returned %d, expected %d",
			prio, tc->prio);
		return;
	}

	for (j = 0; j < tc->len; j++) {
		if (rmsg[j] != smsg[j]) {
			tst_res(TFAIL | TTERRNO, "mq_timedreceive wrong data in loop %d returned %d, expected %d",
			i, rmsg[j], smsg[j]);
			return;
		}
	}

	tst_res(TPASS, "mq_timedreceive returned %ld, priority %u, length: %lu",
			TEST_RETURN, prio, len);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcase),
	.test = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
