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

#include "mq.h"

void create_queue(void)
{
	fd = SAFE_MQ_OPEN(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRWXU, NULL);
}

void create_queue_nonblock(void)
{
	fd = SAFE_MQ_OPEN(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK,
		S_IRWXU, NULL);
}

void create_queue_sig(void)
{
	clock_gettime(CLOCK_REALTIME, &eintr_ts);
	eintr_ts.tv_sec += 3;

	create_queue();
	pid = create_sig_proc(SIGINT, 40, 200000);
}

void create_queue_timeout(void)
{
	clock_gettime(CLOCK_REALTIME, &timeout_ts);
	timeout_ts.tv_nsec += 50000000;
	timeout_ts.tv_sec += timeout_ts.tv_nsec / 1000000000;
	timeout_ts.tv_nsec %= 1000000000;

	create_queue();
}

void open_fd(void)
{
	fd = fd_root;
}

void unlink_queue(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);

	mq_unlink(QUEUE_NAME);
}

void unlink_queue_sig(void)
{
	SAFE_KILL(pid, SIGTERM);
	SAFE_WAIT(NULL);

	unlink_queue();
}

void send_msg(int fd, int len, int prio)
{
	if (mq_timedsend(fd, smsg, len, prio,
		&((struct timespec){0})) < 0)
		tst_brk(TBROK | TERRNO, "mq_timedsend failed");
}
