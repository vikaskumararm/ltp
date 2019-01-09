// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
 */

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_LIBNL_CLI3

#include <string.h>

#include <netlink/cli/utils.h>
#include <netlink/cli/route.h>
#include <netlink/cli/link.h>
#include <linux/rtnetlink.h>

#include "tst_net.h"
#include "tst_safe_stdio.h"

static struct nl_sock *sock;
static struct rtnl_route *route;
static struct nl_cache *link_cache;

static char *carg, *dst, *iface, *ipv6_arg, *nexthop;
static int family = AF_INET;
static int num_loops = 10000;

static void setup(void)
{
	if (tst_parse_int(carg, &num_loops, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of loops '%s'", carg);

	if (ipv6_arg)
		family = AF_INET6;

	if (!iface)
		tst_brk(TBROK, "Missing iface, specify it with -d");

	SAFE_ASPRINTF(&nexthop, "dev=%s", iface);
	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);
	route = nl_cli_route_alloc();
	nl_cli_route_parse_nexthop(route, nexthop, link_cache);
}

static char *tst_ipaddr_un(int ai_family, unsigned int net, unsigned int host)
{
	char *addr, *env, *unused;
	unsigned int max, prefix;

	if (ai_family != AF_INET && ai_family != AF_INET6)
		tst_brk(TCONF, "ai_family must be AF_INET or AF_INET6 (%d)", ai_family);

	if (ai_family == AF_INET) {
		env = "IPV4_NET16_UNUSED";
		max = 255;
		prefix = 24;
	} else {
		env = "IPV6_NET32_UNUSED";
		max = 65535;
		prefix = 64;
	}

	unused = getenv(env);

	if (!unused)
		tst_brk(TCONF, "%s not set (set it with tst_net.sh)", env);

	net %= max;
	host %= max;

	if (ai_family == AF_INET6) {
		if (host > 0 && net > 0)
			SAFE_ASPRINTF(&addr, "%s:%x::%x/%d", unused, net, host, prefix);
		else if (host > 0 && net == 0)
			SAFE_ASPRINTF(&addr, "%s::%x/%d", unused, host, prefix);
		else if (net > 0 && host == 0)
			SAFE_ASPRINTF(&addr, "%s:%x::/%d", unused, net, prefix);
		else
			SAFE_ASPRINTF(&addr, "%s::/%d", unused, prefix);
	} else {
		SAFE_ASPRINTF(&addr, "%s.%d.%d/%d", unused, net, host, prefix);
	}

	return strdup(addr);
}

static void run(void)
{
	int err, i;

	tst_res(TINFO, "Adding and deleting route with different destination");
	for (i = 0; i < num_loops; i++) {
		dst = tst_ipaddr_un(family, i, 0);

		nl_cli_route_parse_dst(route, dst);
		if ((err = rtnl_route_add(sock, route, NLM_F_EXCL)) < 0) {
			tst_res(TFAIL, "Unable to add route to %s via %s: %s",
				dst, nexthop, nl_geterror(err));
			return;
		}

		if ((err = rtnl_route_delete(sock, route, 0)) < 0) {
			tst_res(TFAIL, "Unable to delete route to %s via %s: %s",
				dst, nexthop, nl_geterror(err));
			return;
		}
	}

	tst_res(TPASS, "Routes added and deleted");
}

static struct tst_option options[] = {
	{"6", &ipv6_arg, "-6       Use IPv6 (default is IPv4)"},
	{"c:", &carg, "-c x     Number of loops"},
	{"d:", &iface, "-d IFACE Interface to work on"},
	{NULL, NULL, NULL}
};
static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.setup = setup,
	.options = options,
};
#else
	TST_TEST_TCONF("netlink libraries and headers are required");
#endif /* HAVE_LIBNL_CLI3 */
