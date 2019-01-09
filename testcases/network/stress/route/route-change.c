// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
 */

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_LIBMNL

#include <string.h>

#include <libmnl/libmnl.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <netdb.h>

#include "tst_net.h"
#include "tst_safe_net.h"
#include "tst_safe_stdio.h"

static char *carg, *ciface, *gw_arg, *ipv6_arg;
static int family = AF_INET;
static int num_loops = 10000;
static int iface, is_ipv6;
static unsigned int prefix;
static struct mnl_socket *nl;
static struct addrinfo *remote_addrinfo;
static int fd;

struct in_addr ip;
struct in6_addr ip6;

union {
	in_addr_t ip;
	struct in6_addr ip6;
} dst;

static void setup(void)
{
	if (tst_parse_int(carg, &num_loops, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of loops '%s'", carg);

	prefix = 24;
	if (ipv6_arg) {
		family = AF_INET6;
		is_ipv6 = 1;
		prefix = 64;
	}

	if (!ciface)
		tst_brk(TBROK, "Missing iface, specify it with -d");

	iface = if_nametoindex(ciface);
	if (!iface)
		tst_brk(TBROK, "if_nametoindex failed");
}

static void cleanup(void)
{
	if (fd > 0)
		close(fd);

	if (nl)
		mnl_socket_close(nl);

	if (remote_addrinfo)
		freeaddrinfo(remote_addrinfo);
}

static char *tst_ipaddr_un(int ai_family, unsigned int net, unsigned int host)
{
	char *addr, *env, *unused;
	unsigned int max;

	if (ai_family != AF_INET && ai_family != AF_INET6)
		tst_brk(TCONF, "ai_family must be AF_INET or AF_INET6 (%d)", ai_family);

	if (is_ipv6) {
		env = "IPV6_NET32_UNUSED";
		max = 65535;
	} else {
		env = "IPV4_NET16_UNUSED";
		max = 255;
	}

	unused = getenv(env);

	if (!unused)
		tst_brk(TCONF, "%s not set (set it with tst_net.sh)", env);

	net %= max;
	host %= max;

	if (ai_family == AF_INET6) {
		if (host > 0 && net > 0)
			SAFE_ASPRINTF(&addr, "%s:%x::%x", unused, net, host);
		else if (host > 0 && net == 0)
			SAFE_ASPRINTF(&addr, "%s::%x", unused, host);
		else if (net > 0 && host == 0)
			SAFE_ASPRINTF(&addr, "%s:%x::", unused, net);
		else
			SAFE_ASPRINTF(&addr, "%s::", unused);
	} else {
		SAFE_ASPRINTF(&addr, "%s.%d.%d", unused, net, host);
	}

	return strdup(addr);
}

static void rtnl_route(int iface, char *cdst, uint32_t prefix, char *cgw, int type)
{
	union {
		in_addr_t ip;
		struct in6_addr ip6;
	} dst;
	union {
		in_addr_t ip;
		struct in6_addr ip6;
	} gw;

	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct rtmsg *rtm;
	uint32_t seq, portid;
	int ret;

	if (!inet_pton(family, cdst, &dst))
		tst_brk(TBROK, "inet_pton failed ('%s', errno=%d): %s", cdst, errno, strerror(errno));

	if (cgw && !inet_pton(family, cgw, &gw))
		tst_brk(TBROK, "inet_pton failed ('%s', errno=%d): %s", cgw, errno, strerror(errno));

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= type;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	rtm = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtmsg));
	rtm->rtm_family = family;
	rtm->rtm_dst_len = prefix;
	rtm->rtm_src_len = 0;
	rtm->rtm_tos = 0;
	rtm->rtm_protocol = RTPROT_STATIC;
	rtm->rtm_table = RT_TABLE_MAIN;
	rtm->rtm_type = RTN_UNICAST;
	rtm->rtm_scope = (cgw) ? RT_SCOPE_UNIVERSE : RT_SCOPE_LINK;
	rtm->rtm_flags = 0;

	if (is_ipv6)
		mnl_attr_put(nlh, RTA_DST, sizeof(struct in6_addr), &dst);
	else
		mnl_attr_put_u32(nlh, RTA_DST, dst.ip);

	mnl_attr_put_u32(nlh, RTA_OIF, iface);
	if (cgw) {
		if (is_ipv6)
			mnl_attr_put(nlh, RTA_GATEWAY, sizeof(struct in6_addr),
					&gw.ip6);
		else
			mnl_attr_put_u32(nlh, RTA_GATEWAY, gw.ip);
	}

	nl = mnl_socket_open(NETLINK_ROUTE);
	if (nl == NULL)
		tst_brk(TBROK, "mnl_socket_open failed (errno=%d): %s", errno, strerror(errno));

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
		tst_brk(TBROK, "mnl_socket_bind failed (errno=%d): %s", errno, strerror(errno));

	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
		tst_brk(TBROK, "mnl_socket_sendto failed (errno=%d): %s", errno, strerror(errno));

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret < 0)
		tst_brk(TBROK, "mnl_socket_recvfrom failed (ret=%d, errno=%d): %s", ret, errno, strerror(errno));

	ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
	if (ret < 0)
		tst_brk(TBROK, "mnl_cb_run failed (ret=%d, errno=%d): %s", ret, errno, strerror(errno));

	mnl_socket_close(nl);
}

static void send_udp(char *remote)
{
	// FIXME: sending udp packet broken, but works on 127.0.0.1 => need to
	// create address before
	const char *msg = "foo";
	static char *port;

	fd = SAFE_SOCKET(family, SOCK_DGRAM, IPPROTO_UDP);

	int _port = TST_GET_UNUSED_PORT(family, SOCK_DGRAM);
	SAFE_ASPRINTF(&port, "%d", _port);

	SAFE_SETSOCKOPT(fd, SOL_SOCKET, SO_BINDTODEVICE, ciface,
	strlen(ciface) + 1);
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	hints.ai_addr = INADDR_ANY;

	fprintf(stderr, "%s:%d %s(): original remote: '%s'\n", __FILE__, __LINE__, __func__, remote); // FIXME: debug
	//remote = is_ipv6 ? "fd00:1:1:1::1" : "10.0.0.1"; // FIXME: debug
	fprintf(stderr, "%s:%d %s(): remote: '%s'\n", __FILE__, __LINE__, __func__, remote); // FIXME: debug
	setup_addrinfo(remote, port, &hints, &remote_addrinfo);
	ssize_t len = SAFE_SENDTO(1, fd, msg, strlen(msg), MSG_CONFIRM,
		remote_addrinfo->ai_addr, remote_addrinfo->ai_addrlen);
	fprintf(stderr, "%s:%d %s(): len: %ld\n", __FILE__, __LINE__, __func__, len); // FIXME: debug
}

static void run(void)
{
	int i;
	char *cdst, *cgw = NULL, *remote;

	tst_res(TINFO, "Adding and deleting route with different destination %d times", num_loops);
	for (i = 1; i <= num_loops; i++) {
		if (gw_arg) {
			cdst = tst_ipaddr_un(family, 0, 0);
			cgw = tst_ipaddr_un(family, i, 1);
		} else {
			cdst = tst_ipaddr_un(family, i, 0);
		}
		remote = tst_ipaddr_un(family, i, 3); // FIXME: debug
		fprintf(stderr, "%s:%d %s(): cdst: %s, cgw: %s, remote: %s\n", __FILE__, __LINE__, __func__, cdst, cgw, remote); // FIXME: debug

		rtnl_route(iface, cdst, prefix, cgw, RTM_NEWROUTE);
		fprintf(stderr, "%s:%d %s(): send_udp()\n", __FILE__, __LINE__, __func__); // FIXME: debug
		send_udp(remote);
		rtnl_route(iface, cdst, prefix, cgw, RTM_DELROUTE);
	}

	tst_res(TPASS, "Routes added and deleted");
}

static struct tst_option options[] = {
	{"6", &ipv6_arg, "-6       Use IPv6 (default is IPv4)"},
	{"c:", &carg, "-c x     Number of loops"},
	{"d:", &ciface, "-d IFACE Interface to work on"},
	{"g", &gw_arg, "-g       Change gateway instead of destination"},
	{NULL, NULL, NULL}
};
static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.options = options,
};
#else
	TST_TEST_TCONF("netlink libraries and headers are required");
#endif /* HAVE_LIBNL_CLI3 */
