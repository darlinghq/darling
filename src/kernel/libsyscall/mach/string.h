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

#ifndef _STRING_H_
#define _STRING_H_

#include <stdarg.h>
#include <_types.h>
#include <sys/_types/_null.h>
#include <sys/_types/_size_t.h>
#include <sys/_types/_uintptr_t.h>

// We're purposefully called "string.h" in order to superceed any use
// of Libc's string.h (which no one should be using bar MIG) in order
// to override their use of memcpy.

int _mach_snprintf(char *buffer, int length, const char *fmt, ...) __printflike(3, 4);
int _mach_vsnprintf(char *buffer, int length, const char *fmt, va_list ap) __printflike(3, 0);

// These declarations are just for MIG, other users should include string/strings.h
// These symbols are defined in _libc_funcptr.c

void *memcpy(void *dst0, const void *src0, size_t length);
void *memset(void *dst0, int c0, size_t length);
void bzero(void *dst0, size_t length);

#endif /* _STRING_H_ */
