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
int __coalition_info(uint32_t operation, uint64_t *cid, void *buffer, size_t *bufsize);
int __coalition_ledger(uint32_t operation, uint64_t *cid, void *buffer, size_t *bufsize);

int
coalition_create(uint64_t *cid_out, uint32_t flags)
{
	return __coalition(COALITION_OP_CREATE, cid_out, flags);
}

int
coalition_terminate(uint64_t cid, uint32_t flags)
{
	return __coalition(COALITION_OP_TERMINATE, &cid, flags);
}

int
coalition_reap(uint64_t cid, uint32_t flags)
{
	return __coalition(COALITION_OP_REAP, &cid, flags);
}

int
coalition_info_resource_usage(uint64_t cid, struct coalition_resource_usage *cru, size_t sz)
{
	return __coalition_info(COALITION_INFO_RESOURCE_USAGE, &cid, cru, &sz);
}

int
coalition_info_set_name(uint64_t cid, const char *name, size_t size)
{
	return __coalition_info(COALITION_INFO_SET_NAME, &cid, (void *)name, &size);
}

int
coalition_info_set_efficiency(uint64_t cid, uint64_t flags)
{
	size_t size = sizeof(flags);
	return __coalition_info(COALITION_INFO_SET_EFFICIENCY, &cid, (void *)&flags, &size);
}

int
coalition_ledger_set_logical_writes_limit(uint64_t cid, int64_t limit)
{
	size_t size = sizeof(limit);
	return __coalition_ledger(COALITION_LEDGER_SET_LOGICAL_WRITES_LIMIT, &cid, (void *)&limit, &size);
}
