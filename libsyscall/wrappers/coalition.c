/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <sys/cdefs.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/fcntl.h>
#include <sys/coalition.h>

/* Syscall entry points */
int __coalition(uint32_t operation, uint64_t *cid, uint32_t flags);
int __coalition_info(uint32_t operation, uint64_t *cid, void *buffer, size_t bufsize);

int coalition_create(uint64_t *cid_out, uint32_t flags)
{
	return __coalition(COALITION_OP_CREATE, cid_out, flags);
}

int coalition_terminate(uint64_t cid, uint32_t flags)
{
	return __coalition(COALITION_OP_TERMINATE, &cid, flags);
}

int coalition_reap(uint64_t cid, uint32_t flags)
{
	return __coalition(COALITION_OP_REAP, &cid, flags);
}

int coalition_info_resource_usage(uint64_t cid, struct coalition_resource_usage *cru, size_t sz)
{
	return __coalition_info(COALITION_INFO_RESOURCE_USAGE, &cid, cru, &sz);
}
