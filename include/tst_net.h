// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2019 Petr Vorel <pvorel@suse.cz>
 */

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "tst_safe_stdio.h"

#define MAX_IPV4_PREFIX 32
#define MAX_IPV6_PREFIX 128

#define MAX_IPV4_NET_ID 255
#define MAX_IPV6_NET_ID 65535

#define tst_res_comment(...) { \
	fprintf(stderr, "# "); \
	tst_res(__VA_ARGS__); } \


#define tst_brk_comment(...) { \
	fprintf(stderr, "# "); \
	tst_brk(TCONF, __VA_ARGS__); } \

static inline void print_svar(const char *name, const char *val)
{
	if (name && val)
		printf("export %s=\"%s\"\n", name, val);
}

static inline void print_svar_change(const char *name, const char *val)
{
	if (name && val)
		printf("export %s=\"${%s:-%s}\"\n", name, name, val);
}

/*
 * Function bit_count is from ipcalc project, ipcalc.c.
 */
static inline int bit_count(uint32_t i)
{
	int c = 0;
	unsigned int seen_one = 0;

	while (i > 0) {
		if (i & 1) {
			seen_one = 1;
			c++;
		} else {
			if (seen_one)
				return -1;
		}
		i >>= 1;
	}

	return c;
}

/*
 * Function mask2prefix is from ipcalc project, ipcalc.c.
 */
static inline int mask2prefix(struct in_addr mask)
{
	return bit_count(ntohl(mask.s_addr));
}

/*
 * Function ipv4_mask_to_int is from ipcalc project, ipcalc.c.
 */
static inline int ipv4_mask_to_int(const char *prefix)
{
	int ret;
	struct in_addr in;

	ret = inet_pton(AF_INET, prefix, &in);
	if (ret == 0)
		return -1;

	return mask2prefix(in);
}

/*
 * Function safe_atoi is from ipcalc project, ipcalc.c.
 */
static inline int safe_atoi(const char *s, int *ret_i)
{
	char *x = NULL;
	long l;

	errno = 0;
	l = strtol(s, &x, 0);

	if (!x || x == s || *x || errno)
		return errno > 0 ? -errno : -EINVAL;

	if ((long)(int)l != l)
		return -ERANGE;

	*ret_i = (int)l;

	return 0;
}

/*
 * Function get_prefix use code from ipcalc project, str_to_prefix/ipcalc.c.
 */
static inline int get_prefix(const char *ip_str, int is_ipv6)
{
	char *prefix_str = NULL;
	int prefix = -1, r;

	prefix_str = strchr(ip_str, '/');
	if (!prefix_str)
		return -1;

	*(prefix_str++) = '\0';

	if (!is_ipv6 && strchr(prefix_str, '.'))
		prefix = ipv4_mask_to_int(prefix_str);
	else {
		r = safe_atoi(prefix_str, &prefix);
		if (r != 0)
			tst_brk_comment("conversion error: '%s' is not integer",
					prefix_str);
	}

	if (prefix < 0 || ((is_ipv6 && prefix > MAX_IPV6_PREFIX) ||
		(!is_ipv6 && prefix > MAX_IPV4_PREFIX)))
		tst_brk_comment("bad %s prefix: %s", is_ipv6 ?  "IPv6" : "IPv4",
				prefix_str);

	return prefix;
}

static inline void get_in_addr(const char *ip_str, struct in_addr *ip)
{
	if (inet_pton(AF_INET, ip_str, ip) <= 0)
		tst_brk_comment("bad IPv4 address: '%s'", ip_str);
}

static inline void get_in6_addr(const char *ip_str, struct in6_addr *ip6)
{
	if (inet_pton(AF_INET6, ip_str, ip6) <= 0)
		tst_brk_comment("bad IPv6 address: '%s'", ip_str);
}

static inline void setup_addrinfo(const char *src_addr, const char *port,
			   const struct addrinfo *hints,
			   struct addrinfo **addr_info)
{
	int err = getaddrinfo(src_addr, port, hints, addr_info);

	if (err)
		tst_brk(TBROK, "getaddrinfo failed, %s", gai_strerror(err));

	if (!*addr_info)
		tst_brk(TBROK, "failed to get the address");
}

/*
 * NOTE: unlike shell implementation this support only:
 * tst_ipaddr_un NET_ID HOST_ID
 */
static inline char *tst_ipaddr_un(int ai_family, unsigned int net, unsigned int host)
{
	char *env = "IPV4_NET16_UNUSED";
	unsigned int max = MAX_IPV4_NET_ID;
	char *addr, *unused;

	if (ai_family != AF_INET && ai_family != AF_INET6)
		tst_brk(TCONF, "ai_family must be AF_INET or AF_INET6 (%d)", ai_family);

	if (ai_family == AF_INET6) {
		env = "IPV6_NET32_UNUSED";
		max = MAX_IPV6_NET_ID;
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
