// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) 2019 Martin Doucha <mdoucha@suse.cz>
 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

/*
 * Find valid address for connect()ing to a given bound socket
 * Wildcard addresses like INADDR_ANY will be replaced with localhost
 */
socklen_t get_connect_address(int sock, struct sockaddr_storage *addr);
