/*
 * Copyright (c) Crackerjack Project., 2007-2008, Hitachi, Ltd
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 *
 * Author(s):
 * Takahiro Yasui <takahiro.yasui.mp@hitachi.com>,
 * Yumiko Sugita <yumiko.sugita.yf@hitachi.com>,
 * Satoshi Fujiwara <sa-fuji@sdl.hitachi.co.jp>
 *
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

#ifndef __MQ_H__
#define __MQ_H__

#include "tst_test.h"
#include "tst_sig_proc.h"
#include "tst_safe_posix_ipc.h"

#define MAX_MSGSIZE     8192
#define MSG_LENGTH	 10
#define QUEUE_NAME	"/test_mqueue"

static mqd_t fd, fd_root;
static pid_t pid;
static struct timespec timeout_ts;
static struct timespec eintr_ts;
char smsg[MAX_MSGSIZE];

static inline void create_queue(void)
{
	fd = SAFE_MQ_OPEN(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, 0700, NULL);
}

static inline void create_queue_nonblock(void)
{
	fd = SAFE_MQ_OPEN(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK,
		0700, NULL);
}

static inline void create_queue_sig(void)
{
	clock_gettime(CLOCK_REALTIME, &eintr_ts);
	eintr_ts.tv_sec += 3;

	create_queue();
	pid = create_sig_proc(SIGINT, 40, 200000);
}

static inline void create_queue_timeout(void)
{
	clock_gettime(CLOCK_REALTIME, &timeout_ts);
	timeout_ts.tv_nsec += 50000000;
	timeout_ts.tv_sec += timeout_ts.tv_nsec / 1000000000;
	timeout_ts.tv_nsec %= 1000000000;

	create_queue();
}

static inline void open_fd(void)
{
	fd = fd_root;
}

static inline void unlink_queue(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	mq_unlink(QUEUE_NAME);
}

static inline void unlink_queue_sig(void)
{
	SAFE_KILL(pid, SIGTERM);
	SAFE_WAIT(NULL);

	unlink_queue();
}

static inline void send_msg(int fd, int len, int prio)
{
	if (mq_timedsend(fd, smsg, len, prio,
		&((struct timespec){0})) < 0)
		tst_brk(TBROK | TERRNO, "mq_timedsend failed");
}

#endif /* __MQ_H__ */
