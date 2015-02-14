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

#include "_libkernel_init.h"

extern int mach_init(void);

/* dlsym() funcptr is for legacy support in exc_catcher */
void* (*_dlsym)(void*, const char*) __attribute__((visibility("hidden")));

__attribute__((visibility("hidden")))
_libkernel_functions_t _libkernel_functions;

void
__libkernel_init(_libkernel_functions_t fns,
		const char *envp[] __attribute__((unused)),
		const char *apple[] __attribute__((unused)),
		const struct ProgramVars *vars __attribute__((unused)))
{
	_libkernel_functions = fns;
	if (fns->dlsym) {
		_dlsym = fns->dlsym;
	}
	mach_init();
}
