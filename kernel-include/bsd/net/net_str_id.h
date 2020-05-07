/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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


#ifndef __NSI_STR_ID__
#define __NSI_STR_ID__

#ifdef  KERNEL_PRIVATE

#include <sys/types.h>
#include <sys/kernel_types.h>
#include <sys/queue.h>

struct net_str_id_entry {
	SLIST_ENTRY(net_str_id_entry)   nsi_next;
	u_int32_t                                               nsi_flags;
	u_int32_t                                               nsi_id;
	char                                                    nsi_string[1]; /* variable lenght string */
};

enum {
	NSI_MBUF_TAG    = 0,
	NSI_VENDOR_CODE = 1,
	NSI_IF_FAM_ID   = 2,
	NSI_MAX_KIND
};

extern void net_str_id_first_last(u_int32_t *, u_int32_t *, u_int32_t);

extern errno_t net_str_id_find_internal(const char *, u_int32_t *, u_int32_t, int);

extern void net_str_id_init(void);

#endif /* KERNEL_PRIVATE */

#endif /* __NSI_STR_ID__ */
