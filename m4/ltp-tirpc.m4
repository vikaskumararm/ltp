dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
dnl Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.

AC_DEFUN([LTP_CHECK_TIRPC], [
	dnl libtirpc library and headers
	PKG_CHECK_MODULES([LIBTIRPC], [libtirpc], [have_libtirpc=yes], [have_libtirpc=no])

	dnl libntirpc library and headers
	PKG_CHECK_MODULES([LIBNTIRPC], [libntirpc], [have_libntirpc=yes], [have_libntirpc=no])

	dnl TI-RPC headers (in glibc, since 2.26 installed only when configured with --enable-obsolete-rpc)
	AC_CHECK_HEADERS([rpc/rpc.h], [have_rpc_headers=yes])

	if test "x$have_libtirpc" = "xyes" -o "x$have_libntirpc" = "xyes" -o "x$have_rpc_headers" = "xyes"; then
		AC_DEFINE([HAVE_TIRPC], [1], [Define to 1 if you have libtirpc or libntirpc library and headers or glibc with TI-RPC support])
	fi
])
