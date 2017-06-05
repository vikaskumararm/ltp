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

#include "tst_test.h"
#include "tst_sig_proc.h"
#include "tst_safe_posix_ipc.h"

#define MAX_MSGSIZE     8192
#define MAX_MSG	 10
#define QUEUE_NAME	"/test_mqueue"

mqd_t fd, fd_root;
pid_t pid;
struct sigaction act;
struct timespec timeout_ts;
struct timespec eintr_ts;
char smsg[MAX_MSGSIZE];

void create_queue(void);
void create_queue_nonblock(void);
void create_queue_sig(void);
void create_queue_timeout(void);
void open_fd(void);
void unlink_queue(void);
void unlink_queue_sig(void);
void send_msg(int fd, int len, int prio);
