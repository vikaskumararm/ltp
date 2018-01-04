/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
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
/* The LTP library has some of its own atomic synchronisation primitives
 * contained in this file. Generally speaking these should not be used
 * directly in tests for synchronisation, instead use tst_checkpoint.h,
 * tst_fuzzy_sync.h or the POSIX library.
 *
 * Notes on compile and runtime memory barriers and atomics.
 *
 * Within the LTP library we have three concerns when accessing variables
 * shared by multiple threads or processes:
 *
 * (1) Removal or reordering of accesses by the compiler.
 * (2) Atomicity of addition.
 * (3) LOAD-STORE ordering between threads.
 *
 * The first (1) is the most likely to cause an error if not properly
 * handled. We avoid it by using volatile variables and statements which will
 * not be removed or reordered by the compiler during optimisation. This includes
 * the __atomic and __sync intrinsics and volatile asm statements marked with
 * "memory" as well as variables marked with volatile.
 *
 * On any platform Linux is likely to run on, a LOAD (fetch) or STORE of a
 * 32-bit integer will be atomic. However fetching and adding to a variable is
 * quite likely not; so for (2) we need to ensure we use atomic addition.
 *
 * Finally, for tst_fuzzy_sync at least, we need to ensure that LOADs and
 * STOREs of any shared variables (including non-atomics) that are made
 * between calls to tst_fzsync_wait are completed (globally visible) before
 * tst_fzsync_wait completes. For this, runtime memory and instruction
 * barriers are required in addition to compile time.
 *
 * We use full sequential ordering (__ATOMIC_SEQ_CST) for the sake of
 * simplicity. LTP tests tend to be syscall heavy so any performance gain from
 * using a weaker memory model is unlikely to result in a relatively large
 * performance improvement while at the same time being a potent source of
 * confusion.
 *
 * Likewise, for the fallback ASM, the simplest "definitely will work, always"
 * approach is preferred over anything more performant.
 *
 * Also see Documentation/memory-barriers.txt in the kernel tree and
 * https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
 * terminology may vary between sources.
 */

#ifndef TST_ATOMIC_H__
#define TST_ATOMIC_H__

#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "tst_res_flags.h"

# define LTP_NO_ATOMIC_ERROR "Your compiler does not provide __atomic_add_fetch, " \
	"__sync_add_and_fetch and an LTP implementation is missing for your architecture."

# warning LTP_NO_ATOMIC_ERROR

static inline int tst_atomic_add_return(int i, int *v)
{
	printf(LTP_NO_ATOMIC_ERROR "\n");
	exit(TCONF);
	return -1;
}

static inline int tst_atomic_load(int *v)
{
	printf(LTP_NO_ATOMIC_ERROR "\n");
	exit(TCONF);
	return -1;
}

static inline void tst_atomic_store(int i, int *v)
{
	printf(LTP_NO_ATOMIC_ERROR "\n");
	exit(TCONF);
}

static inline int tst_atomic_inc(int *v)
{
	return tst_atomic_add_return(1, v);
}

static inline int tst_atomic_dec(int *v)
{
	return tst_atomic_add_return(-1, v);
}

#endif	/* TST_ATOMIC_H__ */
