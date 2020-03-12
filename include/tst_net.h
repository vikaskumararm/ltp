// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2019 Petr Vorel <pvorel@suse.cz>
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define MAX_IPV4_PREFIX 32
#define MAX_IPV6_PREFIX 128

#define tst_res_comment(...) { \
	fprintf(stderr, "# "); \
	tst_res(__VA_ARGS__); } \


#define tst_brk_comment(...) { \
	fprintf(stderr, "# "); \
	tst_brk(TCONF, __VA_ARGS__); } \

void tst_print_svar(const char *name, const char *val);
void tst_print_svar_change(const char *name, const char *val);

int tst_bit_count(uint32_t i);
int tst_mask2prefix(struct in_addr mask);
int tst_ipv4_mask_to_int(const char *prefix);
int tst_safe_atoi(const char *s, int *ret_i);
int tst_get_prefix(const char *ip_str, int is_ipv6);
void tst_get_in_addr(const char *ip_str, struct in_addr *ip);
void tst_get_in6_addr(const char *ip_str, struct in6_addr *ip6);

/*
 * Find valid connection address for a given bound socket
 */
socklen_t tst_get_connect_address(int sock, struct sockaddr_storage *addr);

/*
 * Initialize AF_INET/AF_INET6 socket address structure with address and port
 */
void tst_init_sockaddr_inet(struct sockaddr_in *sa, const char *ip_str, uint16_t port);
void tst_init_sockaddr_inet_bin(struct sockaddr_in *sa, uint32_t ip_val, uint16_t port);
void tst_init_sockaddr_inet6(struct sockaddr_in6 *sa, const char *ip_str, uint16_t port);
void tst_init_sockaddr_inet6_bin(struct sockaddr_in6 *sa, const struct in6_addr *ip_val, uint16_t port);
