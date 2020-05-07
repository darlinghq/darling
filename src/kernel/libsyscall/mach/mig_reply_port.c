/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#include <mach/mach.h>
#include <mach/mach_init.h>
#include <sys/cdefs.h>
#include "tsd.h"

__XNU_PRIVATE_EXTERN mach_port_t _task_reply_port = MACH_PORT_NULL;

static inline mach_port_t
_mig_get_reply_port()
{
	return (mach_port_t)(uintptr_t)_os_tsd_get_direct(__TSD_MIG_REPLY);
}

static inline void
_mig_set_reply_port(mach_port_t port)
{
	_os_tsd_set_direct(__TSD_MIG_REPLY, (void *)(uintptr_t)port);
}

/*
 * Called by mig interface code whenever a reply port is needed.
 * Tracing is masked during this call; otherwise, a call to printf()
 * can result in a call to malloc() which eventually reenters
 * mig_get_reply_port() and deadlocks.
 */
mach_port_t
mig_get_reply_port(void)
{
	mach_port_t port = _mig_get_reply_port();
	if (port == MACH_PORT_NULL) {
		port = mach_reply_port();
		_mig_set_reply_port(port);
	}
	return port;
}

/*
 * Called by mig interface code after a timeout on the reply port.
 * May also be called by user. The new mig calls with port passed in.
 */
void
mig_dealloc_reply_port(mach_port_t migport)
{
	mach_port_t port = _mig_get_reply_port();
	if (port != MACH_PORT_NULL && port != _task_reply_port) {
		_mig_set_reply_port(_task_reply_port);
		(void) mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
		if (migport != port) {
			(void) mach_port_deallocate(mach_task_self(), migport);
		}
		_mig_set_reply_port(MACH_PORT_NULL);
	}
}

/*************************************************************
 *  Called by mig interfaces after each RPC.
 *  Could be called by user.
 ***********************************************************/

void
mig_put_reply_port(mach_port_t reply_port __unused)
{
}
