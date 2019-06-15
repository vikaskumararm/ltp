/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2019 Li Wang <liwang@redhat.com>
 */

#ifndef TST_ARCH_H__
#define TST_ARCH_H__

/*
 * Check if test platform is in the given arch list. If yes return 1,
 * otherwise return 0.
 *
 * @arch, NULL or vliad arch list
 */
int tst_on_arch(const char *arch);

#endif /* TST_ARCH_H__ */
