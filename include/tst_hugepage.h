// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Red Hat, Inc.
 */

#ifndef TST_HUGEPAGE__
#define TST_HUGEPAGE__

/*
 * Try to request the specified number of huge pages from system.
 *
 * Note: this depend on the status of system memory fragmentation.
 *       0 - reserve fail
 *       1 - reserve success
 */
int tst_request_hugepages(long hpages);

#endif /* TST_HUGEPAGE_H */
