/*
 * Copyright (c) 2010-2014 Apple Inc. All rights reserved.
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

#ifndef __LIBKERNEL_INIT_H
#define __LIBKERNEL_INIT_H

#include <sys/types.h>
#include <mach/mach.h>
#include <mach/message.h>
#include <mach/mach_types.h>

#ifndef __OS_VOUCHER_PRIVATE__
// We cannot include the actual os/voucher_private.h definition of voucher_t
// here, as that also marks the voucher_XXX functions as exported, which causes
// a compile error when we attempt to mark them hidden in the .c file.
//
// The Libsystem init.c file does include os/voucher_private.h though, as well
// as this file, and this typedef causes an error if it is unguarded.
struct voucher_s;
typedef struct voucher_s *voucher_t;
#endif

typedef const struct _libkernel_functions {
	/* The following functions are included in version 1 of this structure */
	unsigned long version;
	void* (*dlsym)(void*, const char*);
	void* (*malloc)(size_t);
	void  (*free)(void*);
	void* (*realloc)(void*, size_t);
	void  (*_pthread_exit_if_canceled)(int);

	/* The following functions are included in version 2 of this structure */
	void *reserved1;
	void *reserved2;
	void *reserved3;
	void *reserved4;
	void *reserved5;

	/* The following functions are included in version 3 of this structure */
	void (*pthread_clear_qos_tsd)(mach_port_t);

	/* Subsequent versions must only add pointers! */
} *_libkernel_functions_t;

typedef const struct _libkernel_string_functions {
	/* The following functions are included in version 1 of this structure */
	unsigned long version;
	void (*bzero)(void *s, size_t n);
	void * (*memchr)(const void *s, int c, size_t n);
	int (*memcmp)(const void *s1, const void *s2, size_t n);
	void * (*memmove)(void *dst, const void *src, size_t n);
	void * (*memccpy)(void *__restrict dst, const void *__restrict src, int c, size_t n);
	void * (*memset)(void *b, int c, size_t len);
	char * (*strchr)(const char *s, int c);
	int (*strcmp)(const char *s1, const char *s2);
	char * (*strcpy)(char * restrict dst, const char * restrict src);
	size_t (*strlcat)(char * restrict dst, const char * restrict src, size_t maxlen);
	size_t (*strlcpy)(char * restrict dst, const char * restrict src, size_t maxlen);
	size_t (*strlen)(const char *str);
	int (*strncmp)(const char *s1, const char *s2, size_t n);
	char * (*strncpy)(char * restrict dst, const char * restrict src, size_t maxlen);
	size_t (*strnlen)(const char *s, size_t maxlen);
	char * (*strstr)(const char *s, const char *find);
	/* Subsequent versions must only add pointers! */
} *_libkernel_string_functions_t;

typedef const struct _libkernel_voucher_functions {
	/* The following functions are included in version 1 of this structure */
	unsigned long version;
	boolean_t (*voucher_mach_msg_set)(mach_msg_header_t*);
	void (*voucher_mach_msg_clear)(mach_msg_header_t*);
	voucher_mach_msg_state_t (*voucher_mach_msg_adopt)(mach_msg_header_t*);
	void (*voucher_mach_msg_revert)(voucher_mach_msg_state_t);

	/* Subsequent versions must only add pointers! */
} *_libkernel_voucher_functions_t;

struct ProgramVars; /* forward reference */

void __libkernel_init(_libkernel_functions_t fns, const char *envp[],
    const char *apple[], const struct ProgramVars *vars);

kern_return_t __libkernel_platform_init(_libkernel_string_functions_t fns);

kern_return_t __libkernel_voucher_init(_libkernel_voucher_functions_t fns);

#endif // __LIBKERNEL_INIT_H
