// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
 */

#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <linux/dccp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "lapi/udp.h"
#include "lapi/dccp.h"
#include "lapi/netinet_in.h"
#include "lapi/posix_clocks.h"
#include "lapi/socket.h"
#include "lapi/tcp.h"
#include "tst_safe_stdio.h"
#include "tst_safe_pthread.h"
#include "tst_test.h"
#include "tst_net.h"

#define IP_ADDR_DELIMITER " "
#define MAXLINE 1024

static int family = AF_INET;
static int nthreads;
static char *port_path = "port";
static char *ip_path = "ip";
static char *log_path = "route.log";
static char *ready_path = "ready";
static char *cnt_arg, *dev, *ipv6_arg, *server_bg, *source_addr;
static int is_ipv6, cnt;
static pthread_t *threads;
static struct worker *workers;
static int ready;

struct worker {
	int id;
	char *ip;
	int port;
};

static void *server_start(void *p)
{
	struct worker *w = p;
	struct addrinfo hints;
	struct addrinfo *local_addrinfo;
	int cnt = 0, fd, n;
	char *dir;
	char buffer[MAXLINE];
	socklen_t socklen;

	fprintf(stderr, "%s:%d %s(): thread %d, getpid(): %d, getppid(): %d\n", __FILE__, __LINE__, __func__, w->id, getpid(), getppid()); // FIXME: debug
	fd = SAFE_SOCKET(family, SOCK_DGRAM, 0);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	setup_addrinfo(w->ip, "0", &hints, &local_addrinfo);

	tst_res(TINFO, "SO_BINDTODEVICE: to device %s", dev);
	SAFE_SETSOCKOPT(fd, SOL_SOCKET, SO_BINDTODEVICE, dev, strlen(dev) + 1);

	SAFE_BIND(fd, local_addrinfo->ai_addr, local_addrinfo->ai_addrlen);
	int port = TST_GETSOCKPORT(fd);
	tst_res(TINFO, "%d: bind to %s:%d", w->id, w->ip, port);
	fprintf(stderr, "%s:%d %s(): w->id: %d, w->ip: '%s', port: %d, family: %d\n", __FILE__, __LINE__, __func__, w->id, w->ip, port, family); // FIXME: debug
	freeaddrinfo(local_addrinfo);

	// TODO read port and remote from files
	char cwd[PATH_MAX];
	SAFE_GETCWD(cwd, PATH_MAX);
	fprintf(stderr, "%s:%d %s(): before cwd: %s\n", __FILE__, __LINE__, __func__, cwd); // FIXME: debug

	char *file; // FIXME: debug
	SAFE_ASPRINTF(&file, "foo-before-%d", w->id);
	fprintf(stderr, "%s:%d %s(): file: '%s'\n", __FILE__, __LINE__, __func__, file); // FIXME: debug
	SAFE_FILE_PRINTF(file, "---- debug into '%s' ----\n", file); // FIXME: debug

	if (server_bg) {
		SAFE_ASPRINTF(&dir, "%s/%d", server_bg, w->id);
		SAFE_MKDIR(dir, 0700);
		SAFE_CHDIR(dir);

		// TODO read port and remote from files
		SAFE_GETCWD(cwd, PATH_MAX);
		fprintf(stderr, "%s:%d %s(): %d: after cwd: %s\n", __FILE__, __LINE__, __func__, w->id, cwd); // FIXME: debug

		SAFE_FILE_PRINTF(ip_path, "%s", w->ip);
		SAFE_FILE_PRINTF(port_path, "%d", port);
		SAFE_FILE_PRINTF("foo-debug", "---- debug :)' ----\n"); // FIXME: debug

		char *_port = NULL;
		fprintf(stderr, "%s:%d %s(): %d: BEFORE FAILING SCAN\n", __FILE__, __LINE__, __func__, w->id); // FIXME: debug
		//SAFE_FILE_SCANF(port_path, "%s", _port);
		//fprintf(stderr, "%s:%d %s(): scanned port: %s\n", __FILE__, __LINE__, __func__, _port); // FIXME: debug
		int __port;
		SAFE_FILE_SCANF(port_path, "%d", &__port);
		fprintf(stderr, "%s:%d %s(): %d: scanned port: %d\n", __FILE__, __LINE__, __func__, w->id, __port); // FIXME: debug
		SAFE_FILE_PRINTF("foo-port", "scanned port: %d\n", __port); // FIXME: debug

		tst_atomic_inc(&ready);
		SAFE_ASPRINTF(&file, "foo-ready-%d", w->id);
		SAFE_FILE_PRINTF(file, "---- ready '%d' ----\n", ready); // FIXME: debug
	}

	tst_res(TINFO, "%d: waiting for connections", w->id);
	while (1) {
		cnt++;
		int cnt2 = 1; // FIXME: debug

		SAFE_ASPRINTF(&file, "foo-loop-%d.%d.%d", w->id, cnt, cnt2++);
		SAFE_FILE_PRINTF(file, "---- %d ----\n", cnt); // FIXME: debug

		SAFE_ASPRINTF(&file, "foo-loop-%d.%d.%d", w->id, cnt, cnt2++);
		SAFE_FILE_PRINTF(file, "before receiving to %s (%d)\n", cnt > 1 ? buffer : source_addr, cnt); // FIXME: debug

		n = recvfrom(fd, (char *)buffer, MAXLINE,
			     MSG_WAITALL, (struct sockaddr *) local_addrinfo->ai_addr, &socklen);
		buffer[n] = '\0';

		SAFE_ASPRINTF(&file, "foo-loop-%d.%d.%d", w->id, cnt, cnt2++);
		SAFE_FILE_PRINTF(file, "AFTER receiving Client: length: %d, msg: '%s', quit? %s\n", n, buffer, (!strcmp(buffer, "quit") ? "yes" : "no"));
		tst_res(TINFO, "%d: Client: length: %d, msg: '%s'", cnt, n, buffer);

		/*
		if (!strcmp(buffer, "quit")) {
			SAFE_ASPRINTF(&file, "foo-loop-%d.%d.%d", w->id, cnt, cnt2++);
			SAFE_FILE_PRINTF(file, "%d: end of testing\n", cnt); // FIXME: debug
			fprintf(stderr, "%s:%d %s(): EXIT\n", __FILE__, __LINE__, __func__); // FIXME: debug
			exit(55);
			//return;
		}
		SAFE_ASPRINTF(&file, "foo-loop-%d.%d.%d", w->id, cnt, cnt2++);
		SAFE_FILE_PRINTF(file, "%d: continue\n", cnt); // FIXME: debug
		*/

		/*
		   ssize_t len = SAFE_SENDTO(1, fd, buffer, strlen(buffer), MSG_CONFIRM,
		   local_addrinfo->ai_addr, local_addrinfo->ai_addrlen);
		   fprintf(stderr, "%s:%d %s(): AFTER SEND '%s'\n", __FILE__, __LINE__, __func__, buffer); // FIXME: debug
		   SAFE_ASPRINTF(&file, "foo-loop-%d.%d.%d", w->id, cnt, cnt2++);
		   SAFE_FILE_PRINTF(file, "%s:%d %s(): AFTER SEND '%s'\n", __FILE__, __LINE__, __func__, buffer); // FIXME: debug
		   */
		// FIXME: does not make sense to send something and then close socket
		// => use threads
		char *msg = "xxx";
		ssize_t len = SAFE_SENDTO(1, fd, msg, strlen(msg), MSG_CONFIRM,
					  local_addrinfo->ai_addr, local_addrinfo->ai_addrlen);
		fprintf(stderr, "%s:%d %s(): AFTER SEND '%s', len: %ld\n", __FILE__, __LINE__, __func__, msg, len); // FIXME: debug
		SAFE_ASPRINTF(&file, "foo-loop-%d.%d.%d", w->id, cnt, cnt2++);
		SAFE_FILE_PRINTF(file, "%s:%d %s(): AFTER SEND '%s', len: %ld\n", __FILE__, __LINE__, __func__, msg, len); // FIXME: debug

		close(fd);
	}

	return NULL;
}

static void server_init(void)
{
	int i = 0;
	char *tmp = strdup(source_addr);
	char *ptr;

	ptr = strtok(tmp, IP_ADDR_DELIMITER);
	while (ptr != NULL) {
		ptr = strtok(NULL, IP_ADDR_DELIMITER);
		nthreads++;
	}

	if (!nthreads)
		tst_brk(TBROK, "wrong -S parameter ('%s')", source_addr);

	threads = SAFE_MALLOC(sizeof(*threads) * nthreads);
	workers = SAFE_MALLOC(sizeof(struct worker) * nthreads);

	ptr = strtok(source_addr, IP_ADDR_DELIMITER);
	fprintf(stderr, "%s:%d %s(): starting threads, getpid(): %d, getppid(): %d\n", __FILE__, __LINE__, __func__, getpid(), getppid()); // FIXME: debug
	while (ptr != NULL) {
		workers[i].id = (i + 1);
		workers[i].ip = strdup(ptr);
		SAFE_PTHREAD_CREATE(&threads[i], NULL, server_start, &workers[i]);

		ptr = strtok(NULL, IP_ADDR_DELIMITER);
		i++;
	}
	tst_res(TINFO, "running %i servers", nthreads);
}

static void setup(void)
{
	if (ipv6_arg) {
		family = AF_INET6;
		is_ipv6 = 1;
	}

	if (!dev)
		tst_brk(TBROK, "missing interface to bind, specify it with -D");

	if (!source_addr)
		tst_brk(TBROK, "missing source address to bind, specify it with -S");

	if (!cnt_arg)
		tst_brk(TBROK, "missing thread number, specify it with -c");
	if (tst_parse_int(cnt_arg, &cnt, 1, INT_MAX))
		tst_brk(TBROK, "invalid number of threads '%s'", cnt_arg);
}

static void cleanup(void)
{
	if (threads)
		free(threads);

	if (workers)
		free(workers);
}

static void run(void)
{
	int i;

	fprintf(stderr, "%s:%d %s(): BEFORE MOVE\n", __FILE__, __LINE__, __func__); // FIXME: debug
	if (server_bg) {
		tst_res(TINFO, "moving to background");
		move_to_background(log_path);
	}
	fprintf(stderr, "%s:%d %s(): AFTER move_to_background getpid(): %d, getppid(): %d\n", __FILE__, __LINE__, __func__, getpid(), getppid()); // FIXME: debug

	server_init();

	char *file; // FIXME: debug
	i = 0;
	SAFE_ASPRINTF(&file, "foo-main-%d", i);
	SAFE_FILE_PRINTF(file, "---- getpid(): %d, ready: '%d' ----\n", getpid(), ready); // FIXME: debug
	while (ready < nthreads) {
		i++;
		SAFE_ASPRINTF(&file, "foo-main-%d", i);
		SAFE_FILE_PRINTF(file, "---- getpid(): %d, ready: '%d' ----\n", getpid(), ready); // FIXME: debug
		usleep(100000);
	}
	i++;
	SAFE_ASPRINTF(&file, "foo-main-%d-last", i);
	SAFE_FILE_PRINTF(file, "---- LAST getpid(): %d, ready: '%d' ----\n", getpid(), ready); // FIXME: debug
	SAFE_CHDIR(server_bg);
	SAFE_FILE_PRINTF(ready_path, "%d", ready);

	for (i = 0; i < nthreads; i++)
		SAFE_PTHREAD_JOIN(threads[i], NULL);
	tst_res(TFAIL, "TODO: finish implementation"); // FIXME: debug
}

static struct tst_option options[] = {
	{"6", &ipv6_arg, "-6       Use IPv6 (default is IPv4)"},
	{"B:", &server_bg, "-B x     run in background, x - process directory"},
	{"c:", &cnt_arg, "-c x     loop count\n"},
	{"D:", &dev, "-d x     bind to device x\n"},
	{"S:", &source_addr, "-S x     Source address to bind"}, // FIXME: debug
	{NULL, NULL, NULL}
};

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.options = options,
};
