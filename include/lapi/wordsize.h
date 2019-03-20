// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>
 */

#ifndef WORDSIZE_H__
#define WORDSIZE_H__

#ifdef HAVE_BITS_WORDSIZE_H
# include <bits/wordsize.h>
#endif

#ifndef __WORDSIZE
# include <sys/reg.h>
#endif

#endif /* WORDSIZE_H__ */
