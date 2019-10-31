dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) Cisco Systems Inc., 2009
dnl Author: Ngie Cooper <yaneurabeya@gmail.com>

AC_DEFUN([LTP_CHECK_PRCTL_SUPPORT],[
AC_CHECK_HEADERS(sys/prctl.h,[
	AC_CHECK_DECLS([PR_CAPBSET_DROP, PR_CAPBSET_READ, PR_CAP_AMBIENT,
PR_SET_NO_NEW_PRIVS, PR_GET_SPECULATION_CTRL, PR_SET_THP_DISABLE], [],[],[
#include <sys/prctl.h>
]) dnl AC_CHECK_DECLS
])]
)
