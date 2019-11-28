dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (C) 2019 Red Hat, Inc. All Rights Reserved.

AC_DEFUN([LTP_CHECK_FSMOUNT],[
AC_CHECK_FUNCS(fsmount,,)
AC_CHECK_HEADER(sys/mount.h,,,)
])
