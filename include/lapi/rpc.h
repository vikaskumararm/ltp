// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
 */

#ifndef LAPI_RPC_H__
#define LAPI_RPC_H__

#include "config.h"

#ifdef HAVE_LIBTIRPC
# include <tirpc/netconfig.h>
# include <tirpc/rpc/rpc.h>
# include <tirpc/rpc/types.h>
# include <tirpc/rpc/xdr.h>
# include <tirpc/rpc/svc.h>
#else
# include <netconfig.h>
# include <rpc/rpc.h>
# include <rpc/types.h>
# include <rpc/xdr.h>
# include <rpc/svc.h>
#else
# error Missing rpc headers! Install libtirpc!
#endif

#endif	/* LAPI_RPC_H__ */
