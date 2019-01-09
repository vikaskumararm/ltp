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

#include "tst_net.h"
#include "tst_safe_stdio.h"

static char *carg, *cgw, *ciface, *ipv6_arg;
static int family = AF_INET;
static int num_loops = 10000;
static int iface, is_ipv6;
static unsigned int prefix;
static struct mnl_socket *nl;

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
	if (nl)
		mnl_socket_close(nl);
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

	// FIXME: move out of here
	if (!inet_pton(family, cdst, &dst))
		tst_brk(TBROK, "inet_pton failed ('%s', errno=%d): %s", cdst, errno, strerror(errno));

	// FIXME: move out of here
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

static void run(void)
{
	int i;
	char *cdst;

	tst_res(TINFO, "Adding and deleting route with different destination %d times", num_loops);
	for (i = 0; i < num_loops; i++) {
		cdst = tst_ipaddr_un(family, i, 0);
		/*
		if (gw_arg) {
			// FIXME: route-change.c:175: BROK: mnl_cb_run failed (ret=-1, errno=101): Network is unreachable
			cdst = tst_ipaddr_un(family, 0, 0);
			cgw = tst_ipaddr_un(family, i, 1);
		} else {
			cdst = tst_ipaddr_un(family, i, 0);
		}
		*/
		// FIXME: actually create cdst route, to fix -g and for sending packet to it
		fprintf(stderr, "%s:%d %s(): cdst: %s, cgw: %s\n", __FILE__, __LINE__, __func__, cdst, cgw); // FIXME: debug

		rtnl_route(iface, cdst, prefix, cgw, RTM_NEWROUTE);
		rtnl_route(iface, cdst, prefix, cgw, RTM_DELROUTE);
	}

	tst_res(TPASS, "Routes added and deleted");
}

static struct tst_option options[] = {
	{"6", &ipv6_arg, "-6       Use IPv6 (default is IPv4)"},
	{"c:", &carg, "-c x     Number of loops"},
	{"d:", &ciface, "-d IFACE Interface to work on"},
	{"g:", &cgw, "-g      Change gateway instead of destination"},
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
