/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_net.h"
#include "tst_private.h"

void tst_print_svar(const char *name, const char *val)
{
	if (name && val)
		printf("export %s=\"%s\"\n", name, val);
}

void tst_print_svar_change(const char *name, const char *val)
{
	if (name && val)
		printf("export %s=\"${%s:-%s}\"\n", name, name, val);
}

/*
 * Function bit_count is from ipcalc project, ipcalc.c.
 */
static int tst_bit_count(uint32_t i)
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
static int tst_mask2prefix(struct in_addr mask)
{
	return tst_bit_count(ntohl(mask.s_addr));
}

/*
 * Function ipv4_mask_to_int is from ipcalc project, ipcalc.c.
 */
static int tst_ipv4_mask_to_int(const char *prefix)
{
	int ret;
	struct in_addr in;

	ret = inet_pton(AF_INET, prefix, &in);
	if (ret == 0)
		return -1;

	return tst_mask2prefix(in);
}

/*
 * Function safe_atoi is from ipcalc project, ipcalc.c.
 */
static int tst_safe_atoi(const char *s, int *ret_i)
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
int tst_get_prefix(const char *ip_str, int is_ipv6)
{
	char *prefix_str = NULL;
	int prefix = -1, r;

	prefix_str = strchr(ip_str, '/');
	if (!prefix_str)
		return -1;

	*(prefix_str++) = '\0';

	if (!is_ipv6 && strchr(prefix_str, '.'))
		prefix = tst_ipv4_mask_to_int(prefix_str);
	else {
		r = tst_safe_atoi(prefix_str, &prefix);
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

void tst_get_in_addr(const char *ip_str, struct in_addr *ip)
{
	if (inet_pton(AF_INET, ip_str, ip) <= 0)
		tst_brk_comment("bad IPv4 address: '%s'", ip_str);
}

void tst_get_in6_addr(const char *ip_str, struct in6_addr *ip6)
{
	if (inet_pton(AF_INET6, ip_str, ip6) <= 0)
		tst_brk_comment("bad IPv6 address: '%s'", ip_str);
}
