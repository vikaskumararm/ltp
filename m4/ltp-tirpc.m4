dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

AC_DEFUN([LTP_CHECK_TIRPC], [
    PKG_CHECK_MODULES([LIBTIRPC], [libtirpc], [
        AC_DEFINE([HAVE_LIBTIRPC], [1], [Define to 1 if you have libtirpc library and headers])
	], [have_libtirpc=no])
])
