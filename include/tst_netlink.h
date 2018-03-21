/*
 * Copyright (c) 2018 Richard Palethorpe <rpalethorpe@suse.com>
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

#ifndef TST_NETLINK_H
#define TST_NETLINK_H

#include "tst_netlink_fn.h"

/**
 * SAFE_NETLINK_SEND() / tst_safe_netlink_send()
 * @fd: netlink socket file descriptor.
 * @nl_header: netlink header structure describing the message.
 * @payload: an opaque object containing the message data.
 * @payload_len: the @payload length only.
 *
 * Sends a netlink message using safe_sendmsg(). You should set the message
 * type and flags to appropriate values within the @nl_header object. However
 * you do not need to set the message length within @nl_header
 * (nlmsg_len), if you do it will be overwritten.
 *
 * Netlink messages must be aligned correctly which may require padding. This
 * function will add padding if necessary so that you do not need to pad the
 * payload or header structure.
 *
 * See lib/tst_crypto.c for an example.
 *
 * Return: The number of bytes sent.
 */
#define SAFE_NETLINK_SEND(fd, nl_header, payload, payload_len)		\
	tst_safe_netlink_send(__FILE__, __LINE__,			\
			      fd, nl_header, payload, payload_len)

/**
 * SAFE_NETLINK_RECV() / tst_safe_netlink()
 * @fd: netlink socket file descriptor.
 * @nl_header_buf: buffer to contain the received netlink header structure.
 * @buf_len: The length of the header buffer. In general this should be
 * 	     at at least the page size.
 *
 * Receives one or more netlink messages using safe_recvmsg().
 *
 * See lib/tst_crypto.c for an example.
 *
 * Return: The number of bytes received.
 */
#define SAFE_NETLINK_RECV(fd, nl_header_buf, buf_len)			\
	tst_safe_netlink_recv(__FILE__, __LINE__, fd, nl_header_buf, buf_len)

#endif /* TST_NETLINK_H */
