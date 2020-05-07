/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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
#ifndef _LIBKERN_COPYIO_H_
#define _LIBKERN_COPYIO_H_

#include <kern/debug.h>

__BEGIN_DECLS

int     copyin(const user_addr_t uaddr, void *kaddr, size_t len) OS_WARN_RESULT;
int     copyout(const void *kaddr, user_addr_t udaddr, size_t len);

#if defined (_FORTIFY_SOURCE) && _FORTIFY_SOURCE == 0
/* FORTIFY_SOURCE disabled */
#else
OS_ALWAYS_INLINE OS_WARN_RESULT static inline int
__copyin_chk(const user_addr_t uaddr, void *kaddr, size_t len, size_t chk_size)
{
	if (chk_size < len) {
		panic("__copyin_chk object size check failed: uaddr %p, kaddr %p, (%zu < %zu)", (void*)uaddr, kaddr, len, chk_size);
	}
	return copyin(uaddr, kaddr, len);
}

OS_ALWAYS_INLINE static inline int
__copyout_chk(const void *kaddr, user_addr_t uaddr, size_t len, size_t chk_size)
{
	if (chk_size < len) {
		panic("__copyout_chk object size check failed: uaddr %p, kaddr %p, (%zu < %zu)", (void*)uaddr, kaddr, len, chk_size);
	}
	return copyout(kaddr, uaddr, len);
}
#define copyin(uaddr, kaddr, len) __copyin_chk(uaddr, kaddr, len, __builtin_object_size(kaddr, 0))
#define copyout(kaddr, uaddr, len) __copyout_chk(kaddr, uaddr, len, __builtin_object_size(kaddr, 0))
#endif
__END_DECLS
#endif /* _LIBKERN_COPYIO_H_ */
