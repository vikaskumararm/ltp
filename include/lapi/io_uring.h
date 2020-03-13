// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 ARM. All rights reserved.
 * Author: Vikas Kumar <vikas.kumar2@arm.com>
 */

#ifndef IO_URING_H__
#define IO_URING_H__

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdlib.h>

#include "lapi/syscalls.h"

#ifndef IOSQE_FIXED_FILE

/* sqe->flags */
#define IOSQE_FIXED_FILE	(1U << 0)	/* use fixed fileset */
#define IOSQE_IO_DRAIN		(1U << 1)	/* issue after inflight IO */
#define IOSQE_IO_LINK		(1U << 2)	/* links next sqe */
#define IOSQE_IO_HARDLINK	(1U << 3)	/* like LINK, but stronger */

/* io_uring_setup() flags */
#define IORING_SETUP_IOPOLL	(1U << 0)	/* io_context is polled */
#define IORING_SETUP_SQPOLL	(1U << 1)	/* SQ poll thread */
#define IORING_SETUP_SQ_AFF	(1U << 2)	/* sq_thread_cpu is valid */
#define IORING_SETUP_CQSIZE	(1U << 3)	/* app defines CQ size */

/* sqe->fsync_flags */
#define IORING_FSYNC_DATASYNC	(1U << 0)

/* sqe->timeout_flags */
#define IORING_TIMEOUT_ABS	(1U << 0)

/* Magic offsets for the application to mmap the data it needs */
#define IORING_OFF_SQ_RING		0ULL
#define IORING_OFF_CQ_RING		0x8000000ULL
#define IORING_OFF_SQES			0x10000000ULL

/* sq_ring->flags */
#define IORING_SQ_NEED_WAKEUP	(1U << 0) /* needs io_uring_enter wakeup */

/* io_uring_enter(2) flags */
#define IORING_ENTER_GETEVENTS	(1U << 0)
#define IORING_ENTER_SQ_WAKEUP	(1U << 1)

/* io_uring_params->features flags */
#define IORING_FEAT_SINGLE_MMAP		(1U << 0)
#define IORING_FEAT_NODROP		(1U << 1)
#define IORING_FEAT_SUBMIT_STABLE	(1U << 2)

/* io_uring_register(2) opcodes and arguments */
#define IORING_REGISTER_BUFFERS		0
#define IORING_UNREGISTER_BUFFERS	1
#define IORING_REGISTER_FILES		2
#define IORING_UNREGISTER_FILES		3
#define IORING_REGISTER_EVENTFD		4
#define IORING_UNREGISTER_EVENTFD	5
#define IORING_REGISTER_FILES_UPDATE	6

enum {
	IORING_OP_NOP,
	IORING_OP_READV,
	IORING_OP_WRITEV,
	IORING_OP_FSYNC,
	IORING_OP_READ_FIXED,
	IORING_OP_WRITE_FIXED,
	IORING_OP_POLL_ADD,
	IORING_OP_POLL_REMOVE,
	IORING_OP_SYNC_FILE_RANGE,
	IORING_OP_SENDMSG,
	IORING_OP_RECVMSG,
	IORING_OP_TIMEOUT,
	IORING_OP_TIMEOUT_REMOVE,
	IORING_OP_ACCEPT,
	IORING_OP_ASYNC_CANCEL,
	IORING_OP_LINK_TIMEOUT,
	IORING_OP_CONNECT,

	/* this goes last, obviously */
	IORING_OP_LAST,
};

/* IO completion data structure (Completion Queue Entry) */
struct io_uring_cqe {
	uint64_t	user_data;	/* sqe->data submission passed back */
	int32_t 	res;		/* result code for this event */
	uint32_t	flags;
};

/* Filled with the offset for mmap(2) */
struct io_sqring_offsets {
	uint32_t head;
	uint32_t tail;
	uint32_t ring_mask;
	uint32_t ring_entries;
	uint32_t flags;
	uint32_t dropped;
	uint32_t array;
	uint32_t resv1;
	uint64_t resv2;
};

struct io_cqring_offsets {
	uint32_t head;
	uint32_t tail;
	uint32_t ring_mask;
	uint32_t ring_entries;
	uint32_t overflow;
	uint32_t cqes;
	uint64_t resv[2];
};

/* Passed in for io_uring_setup(2). Copied back with updated info on success */
struct io_uring_params {
	uint32_t sq_entries;
	uint32_t cq_entries;
	uint32_t flags;
	uint32_t sq_thread_cpu;
	uint32_t sq_thread_idle;
	uint32_t features;
	uint32_t resv[4];
	struct io_sqring_offsets sq_off;
	struct io_cqring_offsets cq_off;
};


struct io_uring_files_update {
	uint32_t offset;
	uint32_t resv;
	uint64_t __attribute__((aligned(8))) fds;
};

#endif /* IOSQE_FIXED_FILE */


#ifndef HAVE_IO_URING_REGISTER
int io_uring_register(int fd, unsigned int opcode, void *arg,
		      unsigned int nr_args)
{
	return tst_syscall(__NR_io_uring_register, fd, opcode, arg, nr_args);
}
#endif /* HAVE_IO_URING_REGISTER */


#ifndef HAVE_IO_URING_SETUP
int io_uring_setup(unsigned int entries, struct io_uring_params *p)
{
	return tst_syscall(__NR_io_uring_setup, entries, p);
}
#endif /* HAVE_IO_URING_SETUP */

#ifndef HAVE_IO_URING_ENTER
int io_uring_enter(int fd, unsigned int to_submit, unsigned int min_complete,
		   unsigned int flags, sigset_t *sig)
{
	return tst_syscall(__NR_io_uring_enter, fd, to_submit, min_complete,
			flags, sig, _NSIG / 8);
}
#endif /* HAVE_IO_URING_ENTER */



#endif /* IO_URING_H__ */
