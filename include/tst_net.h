// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2019 Petr Vorel <pvorel@suse.cz>
 */

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_IPV4_PREFIX 32
#define MAX_IPV6_PREFIX 128

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

static inline void move_to_background(char *log_path)
{
	fprintf(stderr, "%s:%d %s(): before SAFE_FORK(), getpid(): %d, getppid(): %d\n", __FILE__, __LINE__, __func__, getpid(), getppid()); // FIXME: debug
	if (SAFE_FORK()) {
		fprintf(stderr, "%s:%d %s(): IF SAFE_FORK(), getpid(): %d, getppid(): %d (EXIT THIS ONE)\n", __FILE__, __LINE__, __func__, getpid(), getppid()); // FIXME: debug
		exit(0);
	}
	fprintf(stderr, "%s:%d %s(): AFTER SAFE_FORK(), getpid(): %d, getppid(): %d\n", __FILE__, __LINE__, __func__, getpid(), getppid()); // FIXME: debug

	SAFE_SETSID();

	close(STDIN_FILENO);
	SAFE_OPEN("/dev/null", O_RDONLY);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	int fd = SAFE_OPEN(log_path, O_CREAT | O_TRUNC | O_RDONLY, 00444);

	SAFE_DUP(fd);
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
