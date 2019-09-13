// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Copyright (c) SUSE LLC, 2019
 *  Author: Christian Amann <camann@suse.com>
 */
/*
 * Dummy program used in acct02
 */

#include <unistd.h>
#include <stdio.h> // FIXME: debug

int main(void)
{
	fprintf(stderr, "%s:%d %s(): before sleep\n", __FILE__, __LINE__, __func__); // FIXME: debug
	sleep(1);
	fprintf(stderr, "%s:%d %s(): after sleep\n", __FILE__, __LINE__, __func__); // FIXME: debug
	return 128;
}
