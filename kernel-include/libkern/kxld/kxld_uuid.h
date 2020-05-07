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
#ifndef _KXLD_UUID_H_
#define _KXLD_UUID_H_

#include <sys/types.h>
#if KERNEL
    #include <libkern/kxld_types.h>
#else
    #include "kxld_types.h"
#endif

struct uuid_command;
typedef struct kxld_uuid KXLDuuid;

struct kxld_uuid {
	u_char uuid[16];
	boolean_t has_uuid;
};

/*******************************************************************************
* Constructors and destructors
*******************************************************************************/

void kxld_uuid_init_from_macho(KXLDuuid *uuid, struct uuid_command *src)
__attribute__((nonnull, visibility("hidden")));

void kxld_uuid_clear(KXLDuuid *uuid)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Accessors
*******************************************************************************/

u_long kxld_uuid_get_macho_header_size(void)
__attribute__((pure, visibility("hidden")));

kern_return_t
kxld_uuid_export_macho(const KXLDuuid *uuid, u_char *buf,
    u_long *header_offset, u_long header_size)
__attribute__((pure, nonnull, visibility("hidden")));

#endif /* _KXLD_UUID_H_ */
