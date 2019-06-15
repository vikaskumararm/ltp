/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2019 Li Wang <liwang@redhat.com>
 */

#include <string.h>
#include <stdlib.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_arch.h"
#include "tst_test.h"

static const char *const arch_type_list[] = {
	"i386",
	"x86",
	"x86_64",
	"ia64",
	"ppc",
	"ppc64",
	"s390",
	"s390x",
	"arm",
	"aarch64",
	"sparc",
	NULL
};

static int is_matched_arch(const char *arch, const char *tst_arch)
{
	char *dup_arch, *p;
	const char *delim = " ";

	dup_arch = strdup(arch);

	p = strtok(dup_arch, delim);
	if (strcmp(p, tst_arch) == 0)
		return 1;

	while ((p = strtok(NULL, delim))) {
		if (strcmp(p, tst_arch) == 0)
			return 1;
	}

	free(dup_arch);
	return 0;
}

static int is_valid_arch(const char *arch)
{
	unsigned int i;

	for (i = 0; arch_type_list[i]; i++) {
		if (is_matched_arch(arch, arch_type_list[i]))
			return 1;
	}

	return 0;
}

int tst_on_arch(const char *arch)
{
#if defined(__i386__)
	char *tst_arch = "i386";
#elif defined(__x86__)
	char *tst_arch = "x86";
#elif defined(__x86_64__)
	char *tst_arch = "x86_64";
#elif defined(__ia64__)
	char *tst_arch = "ia64";
#elif defined(__powerpc__)
	char *tst_arch = "ppc";
#elif defined(__powerpc64__)
	char *tst_arch = "ppc64";
#elif defined(__s390__)
	char *tst_arch = "s390";
#elif defined(__s390x__)
	char *tst_arch = "s390x";
#elif defined(__arm__)
	char *tst_arch = "arm";
#elif defined(__arch64__)
	char *tst_arch = "aarch64";
#elif defined(__sparc__)
	char *tst_arch = "sparc";
#endif

	if (arch != NULL && !is_valid_arch(arch))
		tst_brk(TBROK, "please set valid arches!");

	if (arch == NULL || is_matched_arch(arch, tst_arch))
		return 1;

	return 0;
}
