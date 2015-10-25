/*
 * Copyright (c) 2005-2007 Apple Inc. All rights reserved.
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
/*
 * __libc_init() is called from libSystem_initializer()
 */

#include <stdint.h>
#include <pthread.h>
#include <pthread_machdep.h>
#include <machine/cpu_capabilities.h>

struct ProgramVars; /* forward reference */

extern void _program_vars_init(const struct ProgramVars *vars);
extern void _libc_fork_init(void (*prepare)(void), void (*parent)(void), void (*child)(void));
extern void _init_clock_port();
extern pthread_lock_t _malloc_lock;
extern void __xlocale_init(void);
extern void __pthread_pfz_setup(const char *apple[]);
extern void __guard_setup(const char *apple[]);
extern void __malloc_entropy_setup(const char *apple[]);
extern int usenew_impl;

__private_extern__ uintptr_t commpage_pfz_base;

#ifdef PR_5243343
/* 5243343 - temporary hack to detect if we are running the conformance test */
#include <stdlib.h>
__private_extern__ int PR_5243343_flag = 0;
#endif /* PR_5243343 */
__private_extern__ int __pthread_lock_debug = 0;
__private_extern__ int __pthread_lock_old = 0;


void
__libc_init(const struct ProgramVars *vars, void (*atfork_prepare)(void), void (*atfork_parent)(void), void (*atfork_child)(void), const char *apple[])
{
	_program_vars_init(vars);
	_libc_fork_init(atfork_prepare, atfork_parent, atfork_child);
	LOCK_INIT(_malloc_lock);
	_init_clock_port();
	__xlocale_init();
	__guard_setup(apple);
	__pthread_pfz_setup(apple);
	__malloc_entropy_setup(apple);


#ifdef PR_5243343
	/* 5243343 - temporary hack to detect if we are running the conformance test */
	if(getenv("TET_EXECUTE"))
		PR_5243343_flag = 1;
#endif /* PR_5243343 */
#if defined(__i386__) || defined(__x86_64__)
        if(getenv("__PTHREAD_LOCK_DEBUG__"))
                __pthread_lock_debug = 1;
        if(getenv("__PTHREAD_LOCK_OLD__")) {
                __pthread_lock_old = 1;
                usenew_impl = 0;
        }
#endif

}
