dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
dnl Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.

AC_DEFUN([LTP_CHECK_TIRPC], [
	dnl libtirpc library and headers
	PKG_CHECK_MODULES([LIBTIRPC], [libtirpc], [
		have_libtirpc=yes
		TIRPC_CFLAGS=$LIBTIRPC_CFLAGS
		TIRPC_LIBS=$LIBTIRPC_LIBS
	], [have_libtirpc=no])

	dnl TI-RPC headers (in glibc, since 2.26 installed only when configured with --enable-obsolete-rpc)
	AC_CHECK_HEADERS([rpc/rpc.h], [have_rpc_headers=yes])

	if test "x$have_libtirpc" = "xyes" -o "x$have_rpc_headers" = "xyes"; then
		AC_SUBST(HAVE_TIRPC, 1)
		AC_SUBST(TIRPC_CFLAGS)
		AC_SUBST(TIRPC_LIBS)
	fi
])
