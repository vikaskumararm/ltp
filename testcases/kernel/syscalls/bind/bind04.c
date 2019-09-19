// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) 2019 Martin Doucha <mdoucha@suse.cz>
 */

/*
 * Create and bind socket for various standard stream protocols.
 * Then connect to it and send some test data.
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>

#include "tst_test.h"
#include "tst_safe_pthread.h"
#include "libbind.h"

#define SOCKET_FILE "test.sock"
#define ABSTRACT_SOCKET_PATH "\0test.sock"
#define IPV4_ADDRESS "127.0.0.1"
#define IPV6_ADDRESS "::1"
#define BUFFER_SIZE 128

static struct sockaddr_un unix_addr = {
	.sun_family = AF_UNIX,
	.sun_path = SOCKET_FILE
};
static struct sockaddr_un abstract_addr = {
	.sun_family = AF_UNIX,
	.sun_path = ABSTRACT_SOCKET_PATH
};
static struct sockaddr_in ipv4_addr;
static struct sockaddr_in ipv4_any_addr;
static struct sockaddr_in6 ipv6_addr;
static struct sockaddr_in6 ipv6_any_addr;

static struct test_case {
	int type, protocol;
	struct sockaddr *address;
	socklen_t addrlen;
	const char *description;
} testcase_list[] = {
	// UNIX sockets
	{SOCK_STREAM, 0, (struct sockaddr*)&unix_addr, sizeof(unix_addr),
		"AF_UNIX pathname stream"},
	{SOCK_SEQPACKET, 0, (struct sockaddr*)&unix_addr, sizeof(unix_addr),
		"AF_UNIX pathname seqpacket"},
	{SOCK_STREAM, 0, (struct sockaddr*)&abstract_addr,
		sizeof(abstract_addr), "AF_UNIX abstract stream"},
	{SOCK_SEQPACKET, 0, (struct sockaddr*)&abstract_addr,
		sizeof(abstract_addr), "AF_UNIX abstract seqpacket"},

	// IPv4 sockets
	{SOCK_STREAM, 0, (struct sockaddr*)&ipv4_addr, sizeof(ipv4_addr),
		"IPv4 loop TCP variant 1"},
	{SOCK_STREAM, IPPROTO_TCP, (struct sockaddr*)&ipv4_addr,
		sizeof(ipv4_addr), "IPv4 loop TCP variant 2"},
	{SOCK_STREAM, 0, (struct sockaddr*)&ipv4_any_addr,
		sizeof(ipv4_any_addr), "IPv4 any TCP variant 1"},
	{SOCK_STREAM, IPPROTO_TCP, (struct sockaddr*)&ipv4_any_addr,
		sizeof(ipv4_any_addr), "IPv4 any TCP variant 2"},

	// pev
	{SOCK_DCCP, IPPROTO_DCCP, (struct sockaddr*)&ipv4_addr,
		sizeof(ipv4_addr), "pev: IPv4 loop DCCP variant 2"},
	{SOCK_DCCP, IPPROTO_DCCP, (struct sockaddr*)&ipv4_any_addr,
		sizeof(ipv4_any_addr), "pev: IPv4 any DCCP variant 2"},

	{SOCK_STREAM, IPPROTO_SCTP, (struct sockaddr*)&ipv4_addr,
		sizeof(ipv4_addr), "pev: IPv4 loop SCTP variant 2"},
	{SOCK_STREAM, IPPROTO_SCTP, (struct sockaddr*)&ipv4_any_addr,
		sizeof(ipv4_any_addr), "pev: IPv4 any SCTP variant 2"},

	// IPv6 sockets
	{SOCK_STREAM, 0, (struct sockaddr*)&ipv6_addr, sizeof(ipv6_addr),
		"IPv6 loop TCP variant 1"},
	{SOCK_STREAM, IPPROTO_TCP, (struct sockaddr*)&ipv6_addr,
		sizeof(ipv6_addr), "IPv6 loop TCP variant 2"},
	{SOCK_STREAM, 0, (struct sockaddr*)&ipv6_any_addr,
		sizeof(ipv6_any_addr), "IPv6 any TCP variant 1"},
	{SOCK_STREAM, IPPROTO_TCP, (struct sockaddr*)&ipv6_any_addr,
		sizeof(ipv6_any_addr), "IPv6 any TCP variant 2"}
};

static void setup(void)
{
	srand(time(0));

	// Configure listen address for AF_INET test cases
	memset(&ipv4_addr, 0, sizeof(ipv4_addr));
	ipv4_addr.sin_family = AF_INET;
	ipv4_addr.sin_port = htons(0);
	ipv4_addr.sin_addr.s_addr = inet_addr(IPV4_ADDRESS);

	memset(&ipv4_any_addr, 0, sizeof(ipv4_any_addr));
	ipv4_any_addr.sin_family = AF_INET;
	ipv4_any_addr.sin_port = htons(0);
	ipv4_any_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Configure listen address for AF_INET6 test cases
	memset(&ipv6_addr, 0, sizeof(ipv6_addr));
	ipv6_addr.sin6_family = AF_INET6;
	ipv6_addr.sin6_port = htons(0);
	memcpy(&ipv6_addr.sin6_addr, &in6addr_loopback,
		sizeof(struct in6_addr));

	memset(&ipv6_any_addr, 0, sizeof(ipv6_any_addr));
	ipv6_any_addr.sin6_family = AF_INET6;
	ipv6_any_addr.sin6_port = htons(0);
	memcpy(&ipv6_any_addr.sin6_addr, &in6addr_any,
		sizeof(struct in6_addr));
}

static void *peer_thread(void *tc_ptr)
{
	const struct test_case *tc = (struct test_case*)tc_ptr;
	int sock;
	unsigned request;
	const char *response;

	sock = SAFE_SOCKET(tc->address->sa_family, tc->type, tc->protocol);
	SAFE_CONNECT(sock, tc->address, tc->addrlen);
	SAFE_READ(1, sock, &request, sizeof(request));

	if (request < ARRAY_SIZE(testcase_list)) {
		response = testcase_list[request].description;
	} else {
		response = "Invalid request value";
	}

	SAFE_WRITE(1, sock, response, strlen(response) + 1);
	SAFE_CLOSE(sock);
	return NULL;
}

static void test_bind(unsigned int n)
{
	struct test_case tc_copy, *tc = testcase_list + n;
	struct sockaddr_storage listen_addr, remote_addr;
	struct sockaddr_un *tmp_addr;
	socklen_t remote_len = sizeof(struct sockaddr_storage);
	int listen_sock, sock, size;
	unsigned rand_index;
	pthread_t thread_id;
	char buffer[BUFFER_SIZE];
	const char *exp_data;

	// Create listen socket
	tst_res(TINFO, "Testing %s", tc->description);
	listen_sock = SAFE_SOCKET(tc->address->sa_family, tc->type,
		tc->protocol);

	TEST(bind(listen_sock, tc->address, tc->addrlen));

	if (TST_RET) {
		tst_res(TFAIL | TERRNO, "bind() failed");
		close(listen_sock);
		return;
	}

	// IPv4/IPv6 tests use wildcard addresses, resolve a valid connection
	// address for peer thread
	memcpy(&tc_copy, tc, sizeof(struct test_case));
	tc_copy.addrlen = get_connect_address(listen_sock, &listen_addr);
	tc_copy.address = (struct sockaddr*)&listen_addr;

	// Start peer thread and wait for connection
	SAFE_LISTEN(listen_sock, 1);
	SAFE_PTHREAD_CREATE(&thread_id, NULL, peer_thread, &tc_copy);
	sock = accept(listen_sock, (struct sockaddr*)&remote_addr,
		&remote_len);

	if (sock < 0) {
		tst_brk(TBROK | TERRNO, "accept() failed");
	}

	// Send request
	rand_index = rand() % ARRAY_SIZE(testcase_list);
	SAFE_WRITE(1, sock, &rand_index, sizeof(rand_index));

	// Read response
	size = SAFE_READ(0, sock, buffer, BUFFER_SIZE - 1);
	buffer[size] = '\0';
	exp_data = testcase_list[rand_index].description;

	if (!strcmp(buffer, exp_data)) {
		tst_res(TPASS, "Communication successful");
	} else {
		tst_res(TFAIL, "Received invalid data. Expected: \"%s\". "
			"Received: \"%s\"", exp_data, buffer);
	}

	// Cleanup
	SAFE_CLOSE(sock);
	SAFE_CLOSE(listen_sock);
	pthread_join(thread_id, NULL);
	tmp_addr = (struct sockaddr_un*)tc->address;

	if (tc->address->sa_family == AF_UNIX && tmp_addr->sun_path[0]) {
		SAFE_UNLINK(tmp_addr->sun_path);
	}
}

static struct tst_test test = {
	.test = test_bind,
	.tcnt = ARRAY_SIZE(testcase_list),
	.needs_tmpdir = 1,
	.setup = setup,
};
