// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) 2019 Martin Doucha <mdoucha@suse.cz>
 */

#include <string.h>

#include "tst_safe_net.h"
#include "libbind.h"

socklen_t get_connect_address(int sock, struct sockaddr_storage *addr)
{
	struct sockaddr_in *inet_ptr;
	struct sockaddr_in6 *inet6_ptr;
	size_t tmp_size;
	socklen_t ret = sizeof(*addr);

	SAFE_GETSOCKNAME(sock, (struct sockaddr*)addr, &ret);

	// Sanitize wildcard addresses
	switch (addr->ss_family) {
	case AF_INET:
		inet_ptr = (struct sockaddr_in*)addr;

		switch (ntohl(inet_ptr->sin_addr.s_addr)) {
		case INADDR_ANY:
		case INADDR_BROADCAST:
			inet_ptr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			break;
		}

		break;

	case AF_INET6:
		inet6_ptr = (struct sockaddr_in6*)addr;
		tmp_size = sizeof(struct in6_addr);

		if (!memcmp(&inet6_ptr->sin6_addr, &in6addr_any, tmp_size)) {
			memcpy(&inet6_ptr->sin6_addr, &in6addr_loopback,
				tmp_size);
		}

		break;
	}

	return ret;
}
