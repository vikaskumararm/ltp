dnl Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

AC_DEFUN([LTP_CHECK_LIBNL], [
    PKG_CHECK_MODULES([LIBNL_CLI3], [libnl-cli-3.0], [
        AC_DEFINE([HAVE_LIBNL_CLI3], [1], [Define to 1 if you have netlink libraries and headers])
	], [have_libnl3=no])
])
