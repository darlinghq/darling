/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
//
//	wipefs.h
//
#ifndef WIPEFS_H
#define WIPEFS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __wipefs_ctx *wipefs_ctx;

__BEGIN_DECLS
extern int wipefs_alloc(int fd, size_t block_size, wipefs_ctx *handle);
extern int wipefs_include_blocks(wipefs_ctx handle, off_t block_offset, off_t nblocks);
extern int wipefs_except_blocks(wipefs_ctx handle, off_t block_offset, off_t nblocks);
extern int wipefs_wipe(wipefs_ctx handle);
extern void wipefs_free(wipefs_ctx *handle);
__END_DECLS

#ifdef __cplusplus
}
#endif

#endif // #ifndef WIPEFS_H

