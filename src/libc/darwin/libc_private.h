/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef __LIBC_PRIVATE_H__
#define __LIBC_PRIVATE_H__

#include <sys/cdefs.h>
#include <Availability.h>
#include <stddef.h>

__BEGIN_DECLS

struct _libc_functions {
	unsigned long version;
	void (*atfork_prepare)(void); // version 1
	void (*atfork_parent)(void); // version 1
	void (*atfork_child)(void); // version 1
	char *(*dirhelper)(int, char *, size_t); // version 1
};

struct ProgramVars; // forward reference

__deprecated_msg("use _libc_initializer()")
extern void
__libc_init(const struct ProgramVars *vars,
	void (*atfork_prepare)(void),
	void (*atfork_parent)(void),
	void (*atfork_child)(void),
	const char *apple[]);

__OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0)
extern void
_libc_initializer(const struct _libc_functions *funcs,
	const char *envp[],
	const char *apple[],
	const struct ProgramVars *vars);

extern void
_libc_fork_child(void);

extern int
_atexit_receipt(void);

__IOS_AVAILABLE(9.0) __OSX_AVAILABLE(10.11)
extern void
abort_report_np(const char *, ...) __dead2 __printflike(1, 2);

__END_DECLS

#endif // __LIBC_PRIVATE_H__
