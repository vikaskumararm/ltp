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

#define MAXLINE 1024

static char *carg, *ciface, *gw_arg, *ipv6_arg;
static int family = AF_INET;
static int num_loops = 10000;
static int iface, is_ipv6;
static unsigned int prefix;
static struct mnl_socket *nl;
static struct addrinfo *local_addrinfo;
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

static void send_udp(char *local, char *remote)
{
	fd = SAFE_SOCKET(family, SOCK_DGRAM, IPPROTO_UDP);

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	hints.ai_addr = INADDR_ANY;
	setup_addrinfo(local, NULL, &hints, &local_addrinfo);
	char *port;
	int _port = TST_GET_UNUSED_PORT(family, SOCK_DGRAM);
	SAFE_ASPRINTF(&port, "%d", _port);
	setup_addrinfo(remote, port, &hints, &remote_addrinfo);

	SAFE_BIND(fd, local_addrinfo->ai_addr, local_addrinfo->ai_addrlen);

	SAFE_SENDTO(1, fd, remote, strlen(remote), MSG_CONFIRM,
		remote_addrinfo->ai_addr, remote_addrinfo->ai_addrlen);

	close(fd);
}

static void run(void)
{
	int i;
	char *cdst, *cgw = NULL, *local, *remote;

	tst_res(TINFO, "Adding and deleting route with different destination %d times", num_loops);
	for (i = 1; i <= num_loops; i++) {
		local = tst_ipaddr_un(family, i, 2);
		remote = tst_ipaddr_un(family, i, 3);
		if (gw_arg) {
			/*
			 * 00:35:48.955345 IP 10.23.1.2.56218 > 10.23.1.3.30434: UDP, length 9
			 * 00:35:48.955874 IP 10.23.2.2.47105 > 10.23.2.3.36016: UDP, length 9
			 * 00:35:48.956222 IP 10.23.3.2.49432 > 10.23.3.3.47336: UDP, length 9
			 *
			 * 00:36:25.257693 IP6 fd00:23:1::2.48503 > fd00:23:1::3.63398: UDP, length 12
			 * 00:36:25.258325 IP6 fd00:23:2::2.38765 > fd00:23:2::3.14557: UDP, length 12
			 * 00:36:25.259503 IP6 fd00:23:3::2.38089 > fd00:23:3::3.18413: UDP, length 12
			 */
			cdst = tst_ipaddr_un(family, 0, 0);
			cgw = tst_ipaddr_un(family, i, 1);
		} else {
			/*
			 * 00:37:22.231789 IP 10.23.1.2.55149 > 10.23.1.3.16316: UDP, length 9
			 * 00:37:22.232212 IP 10.23.2.2.52751 > 10.23.2.3.50852: UDP, length 9
			 * 00:37:22.232452 IP 10.23.3.2.52483 > 10.23.3.3.44747: UDP, length 9
			 *
			 * 00:36:49.043514 IP6 fd00:23:1::2.35133 > fd00:23:1::3.60570: UDP, length 12
			 * 00:36:49.044272 IP6 fd00:23:2::2.52313 > fd00:23:2::3.5292: UDP, length 12
			 * 00:36:49.045089 IP6 fd00:23:3::2.53396 > fd00:23:3::3.42720: UDP, length 12
			 */
			cdst = tst_ipaddr_un(family, i, 0);
		}
		fprintf(stderr, "%s:%d %s(): i=%d: cdst: %s, cgw: %s, remote: %s, prefix: %d\n", __FILE__, __LINE__, __func__, i, cdst, cgw, remote, prefix); // FIXME: debug

		rtnl_route(iface, cdst, prefix, cgw, RTM_NEWROUTE);
		send_udp(local, remote);
		rtnl_route(iface, cdst, prefix, cgw, RTM_DELROUTE);
	}

	tst_res(TPASS, "Routes worked correctly");
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
