/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#include <sys/cdefs.h>

typedef enum {
	KDP_EVENT_ENTER,
	KDP_EVENT_EXIT,
	KDP_EVENT_PANICLOG
} kdp_event_t;

__BEGIN_DECLS
typedef void (*kdp_callout_fn_t)(void *arg, kdp_event_t event);

/*
 * Register fn(arg, event) to be called at kdp entry/exit.
 * The callouts are made in a single-threaded environment, interrupts are
 * disabled and processors other than the callout processor quiesced.
 * N.B. callouts are strictly limited in what they can do: they must execute
 * with interrupts disabled and they can't call back into the kernel for any
 * non-trivial service.
 */
extern void kdp_register_callout(kdp_callout_fn_t fn, void *arg);

__END_DECLS
