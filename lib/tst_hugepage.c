// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Red Hat, Inc.
 */

#define TST_NO_DEFAULT_MAIN

#include "tst_test.h"
#include "tst_hugepage.h"

int tst_request_hugepages(long hpages)
{
	int val;
	long mem_avail, max_hpages;

	if (FILE_LINES_SCANF("/proc/meminfo",
				"MemAvailable: %ld", &mem_avail)) {
		/*
		 * Dropping caches and using "MemFree:" on kernel
		 * that doesn't have "MemAvailable:" in Meminfo
		 */
		tst_res(TINFO, "MemAvailable: not found in /proc/meminfo");

		SAFE_FILE_PRINTF("/proc/sys/vm/drop_caches", "3");
		mem_avail = SAFE_READ_MEMINFO("MemFree:");
	}

	max_hpages = mem_avail / SAFE_READ_MEMINFO("Hugepagesize:");

	if (hpages > max_hpages) {
		tst_res(TINFO, "Request %ld hugepages failed, memory too fragmented? "
				"The max hugepage available count %ld",
				hpages, max_hpages);
		return 0;
	}

	SAFE_FILE_PRINTF("/proc/sys/vm/nr_hugepages", "%ld", hpages);
	SAFE_FILE_SCANF("/proc/sys/vm/nr_hugepages", "%d", &val);
	if (val != hpages)
		tst_brk(TBROK, "nr_hugepages = %d, but expect %ld", val, hpages);

	return 1;
}
