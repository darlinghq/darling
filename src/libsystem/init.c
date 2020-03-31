/*
 * Copyright (c) 2007, 2008, 2011-2013 Apple Inc. All rights reserved.
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

#include <TargetConditionals.h>	// for TARGET_OS_*

#include <stddef.h>
#include <stdlib.h>
#include <libc_private.h>
#include <pthread.h>
#include <pthread/private.h>
#include <dlfcn.h>
#include <errno.h>
#include <_libkernel_init.h> // Must be after voucher_private.h

#include <mach-o/dyld_priv.h>

// system library initialisers
extern void bootstrap_init(void);		// from liblaunch.dylib
extern void mach_init(void);			// from libsystem_kernel.dylib
extern void __libplatform_init(void *future_use, const char *envp[], const char *apple[], const struct ProgramVars *vars);
extern void __pthread_init(const struct _libpthread_functions *libpthread_funcs, const char *envp[], const char *apple[], const struct ProgramVars *vars);	// from libsystem_pthread.dylib
extern void __malloc_init(const char *apple[]); // from libsystem_malloc.dylib
extern void __keymgr_initializer(void);		// from libkeymgr.dylib
extern void _dyld_initializer(void);		// from libdyld.dylib
extern void libdispatch_init(void);		// from libdispatch.dylib
extern void _libxpc_initializer(void);		// from libxpc.dylib
extern void _libsecinit_initializer(void);        // from libsecinit.dylib
extern void _libtrace_init(void);		// from libsystem_trace.dylib


// signal malloc stack logging that initialisation has finished
extern void __stack_logging_early_finished(void); // form libsystem_c.dylib

// clear qos tsd (from pthread)
extern void _pthread_clear_qos_tsd(mach_port_t) __attribute__((weak_import));

// system library atfork handlers
extern void _pthread_fork_prepare(void);
extern void _pthread_fork_parent(void);
extern void _pthread_fork_child(void);
extern void _pthread_fork_child_postinit(void);
extern void _pthread_exit_if_canceled(int);

extern void dispatch_atfork_prepare(void);
extern void dispatch_atfork_parent(void);
extern void dispatch_atfork_child(void);

extern void _libtrace_fork_child(void);

extern void _malloc_fork_prepare(void);
extern void _malloc_fork_parent(void);
extern void _malloc_fork_child(void);

extern void _mach_fork_child(void);
extern void _mach_fork_parent(void);
extern void _notify_fork_child(void);
extern void _dyld_fork_child(void);
extern void xpc_atfork_prepare(void);
extern void xpc_atfork_parent(void);
extern void xpc_atfork_child(void);
extern void _libSC_info_fork_prepare(void);
extern void _libSC_info_fork_parent(void);
extern void _libSC_info_fork_child(void);
extern void _asl_fork_child(void);

#if defined(HAVE_SYSTEM_CORESERVICES)
// libsystem_coreservices.dylib
extern void _libcoreservices_fork_child(void);
extern char *_dirhelper(int, char *, size_t);
#endif

#if TARGET_OS_EMBEDDED && !TARGET_OS_WATCH && !__LP64__
extern void _vminterpose_init(void);
#endif

// advance decls for below;
void libSystem_atfork_prepare(void);
void libSystem_atfork_parent(void);
void libSystem_atfork_child(void);

// libsyscall_initializer() initializes all of libSystem.dylib
// <rdar://problem/4892197>
__attribute__((constructor))
static void
libSystem_initializer(int argc,
		      const char* argv[],
		      const char* envp[],
		      const char* apple[],
		      const struct ProgramVars* vars)
{
	static const struct _libkernel_functions libkernel_funcs = {
		.version = 3,
		// V1 functions
		.dlsym = dlsym,
		.malloc = malloc,
		.free = free,
		.realloc = realloc,
		._pthread_exit_if_canceled = _pthread_exit_if_canceled,
		// V2 functions (removed)
		// V3 functions
		.pthread_clear_qos_tsd = _pthread_clear_qos_tsd,
		.dyld_func_lookup = _dyld_func_lookup,
	};

	static const struct _libpthread_functions libpthread_funcs = {
		.version = 2,
		.exit = exit,
		.malloc = malloc,
		.free = free,
	};
	
	static const struct _libc_functions libc_funcs = {
		.version = 1,
		.atfork_prepare = libSystem_atfork_prepare,
		.atfork_parent = libSystem_atfork_parent,
		.atfork_child = libSystem_atfork_child,
#if defined(HAVE_SYSTEM_CORESERVICES)
		.dirhelper = _dirhelper,
#endif
	};

	__libkernel_init(&libkernel_funcs, envp, apple, vars);

	__libplatform_init(NULL, envp, apple, vars);

	__pthread_init(&libpthread_funcs, envp, apple, vars);

	_libc_initializer(&libc_funcs, envp, apple, vars);

	// TODO: Move __malloc_init before __libc_init after breaking malloc's upward link to Libc
	__malloc_init(apple);

#if !TARGET_OS_SIMULATOR && !TARGET_OS_TV && !TARGET_OS_WATCH
	/* <rdar://problem/9664631> */
	__keymgr_initializer();
#endif

	_dyld_initializer();

	libdispatch_init();
	_libxpc_initializer();

#if !(TARGET_OS_EMBEDDED || TARGET_OS_SIMULATOR)
	// _libsecinit_initializer(); // Darling: not yet
#endif

	__stack_logging_early_finished();

#if TARGET_OS_EMBEDDED && !TARGET_OS_WATCH && !__LP64__
	// _vminterpose_init(); // Darling: not yet
#endif

	// _libtrace_init(); // must be initialized after dispatch // Darling: not yet

#if !TARGET_OS_IPHONE
    /* <rdar://problem/22139800> - Preserve the old behavior of apple[] for
     * programs that haven't linked against newer SDK.
	 */
#define APPLE0_PREFIX "executable_path="
	if (dyld_get_program_sdk_version() < DYLD_MACOSX_VERSION_10_11){
		if (strncmp(apple[0], APPLE0_PREFIX, strlen(APPLE0_PREFIX)) == 0){
			apple[0] = apple[0] + strlen(APPLE0_PREFIX);
		}
	}
#endif

	/* <rdar://problem/11588042>
	 * C99 standard has the following in section 7.5(3):
	 * "The value of errno is zero at program startup, but is never set
	 * to zero by any library function."
	 */
	errno = 0;
}

/*
 * libSystem_atfork_{prepare,parent,child}() are called by libc during fork(2).
 * They call the corresponding atfork handlers for other libsystem components.
 */
void
libSystem_atfork_prepare(void)
{
	// _libSC_info_fork_prepare(); // Darling: not yet
	xpc_atfork_prepare();
	dispatch_atfork_prepare();
	_pthread_fork_prepare();
	_malloc_fork_prepare();
}

void
libSystem_atfork_parent(void)
{
	_malloc_fork_parent();
	_pthread_fork_parent();
	dispatch_atfork_parent();
	xpc_atfork_parent();
	_mach_fork_parent();
	// _libSC_info_fork_parent(); // Darling: not yet
}

void
libSystem_atfork_child(void)
{
	// Darling: change
	_mach_fork_child();
	_mig_fork_child();

	_dyld_fork_child();
	_pthread_fork_child();
	_malloc_fork_child();
	dispatch_atfork_child();
	
	// _mach_fork_child();
	_libc_fork_child();

#if defined(HAVE_SYSTEM_CORESERVICES)
	_libcoreservices_fork_child();
#endif

	_asl_fork_child();
	_notify_fork_child();
	xpc_atfork_child();
	// _libSC_info_fork_child(); // Darling: not yet

	_pthread_fork_child_postinit();
	// _libtrace_fork_child(); // no prep work required for the fork
	kqueue_atfork();
}

/*  
 *  Old crt1.o glue used to call through mach_init_routine which was used to initialize libSystem.
 *  LibSystem now auto-initializes but mach_init_routine is left for binary compatibility.
 */
static void mach_init_old(void) {}
void (*mach_init_routine)(void) = &mach_init_old;

/*
 *	This __crashreporter_info__ symbol is for all non-dylib parts of libSystem.
 */
const char *__crashreporter_info__;
asm (".desc __crashreporter_info__, 0x10");
