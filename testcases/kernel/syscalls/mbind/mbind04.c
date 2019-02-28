// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * We are testing mbind() with MPOL_BIND and MPOL_PREFERRED.
 *
 * For each node with memory we set its bit in nodemask with set_mempolicy()
 * and verify that memory has been faulted accordingly.
 */

#include <errno.h>
#include "config.h"
#ifdef HAVE_NUMA_H
# include <numa.h>
# include <numaif.h>
# include "mbind.h"
#endif
#include "tst_test.h"
#include "tst_numa.h"

#ifdef HAVE_NUMA_V2

static size_t page_size;
static struct tst_nodemap *nodes;

#define PAGES_ALLOCATED 16u

static void setup(void)
{
	page_size = getpagesize();

	nodes = tst_get_nodemap(TST_NUMA_MEM, 2 * PAGES_ALLOCATED * page_size / 1024);
	if (nodes->cnt <= 1)
		tst_brk(TCONF, "Test requires at least two NUMA memory nodes");
}

static void cleanup(void)
{
	tst_nodemap_free(nodes);
}

static void verify_policy(unsigned int node, int mode, unsigned flag)
{
	struct bitmask *bm = numa_allocate_nodemask();
	unsigned int i;
	void *ptr;
	unsigned long size = PAGES_ALLOCATED * page_size;

	numa_bitmask_setbit(bm, node);

	ptr = tst_numa_map(NULL, size);

	TEST(mbind(ptr, size, mode, bm->maskp, bm->size + 1, flag));

	if (TST_RET) {
		tst_res(TFAIL | TTERRNO,
		        "mbind(%s, %s) node %u",
		        tst_numa_mode_name(mode), mbind_flag_name(flag), node);
		return;
	}

	tst_res(TPASS, "mbind(%s, %s) node %u",
	        tst_numa_mode_name(mode), mbind_flag_name(flag), node);

	numa_free_nodemask(bm);

	const char *prefix = "child: ";

	if (SAFE_FORK()) {
		prefix = "parent: ";
		tst_reap_children();
	}

	tst_nodemap_reset_counters(nodes);
	tst_numa_fault(ptr, size);
	tst_nodemap_count_pages(nodes, ptr, size);
	tst_numa_unmap(ptr, size);
	tst_nodemap_print_counters(nodes);

	for (i = 0; i < nodes->cnt; i++) {
		if (nodes->map[i] == node) {
			if (nodes->counters[i] == PAGES_ALLOCATED) {
				tst_res(TPASS, "%sNode %u allocated %u",
				        prefix, node, PAGES_ALLOCATED);
			} else {
				tst_res(TFAIL, "%sNode %u allocated %u, expected %u",
				        prefix, node, nodes->counters[i],
				        PAGES_ALLOCATED);
			}
			continue;
		}

		if (nodes->counters[i]) {
			tst_res(TFAIL, "%sNode %u allocated %u, expected 0",
			        prefix, i, nodes->counters[i]);
		}
	}
}

static void verify_mbind(unsigned int n)
{
	unsigned int i;
	int mode = n ? MPOL_PREFERRED : MPOL_BIND;

	for (i = 0; i < nodes->cnt; i++) {
		verify_policy(nodes->map[i], mode, 0);
		verify_policy(nodes->map[i], mode, MPOL_MF_STRICT);
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_mbind,
	.tcnt = 2,
	.forks_child = 1,
	.needs_checkpoints = 1,
};

#else

TST_TEST_TCONF(NUMA_ERROR_MSG);

#endif /* HAVE_NUMA_V2 */
