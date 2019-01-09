# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Copyright (c) Linux Test Project, 2006-2019
# Ngie Cooper, October 2009

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/testcases.mk

INSTALL_TARGETS		+= route[4-6]-* *.sh

route-change: CFLAGS += $(LIBMNL_CFLAGS)
route-change: LDLIBS += $(LIBMNL_LIBS)

route-change-server: CFLAGS += -pthread

include $(top_srcdir)/include/mk/generic_leaf_target.mk
