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
 * _libc_initializer() is called from libSystem_initializer()
 */

#include <crt_externs.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <machine/cpu_capabilities.h>
#include <TargetConditionals.h>
#include <_simple.h>

#include "libc_private.h"

extern void _program_vars_init(const struct ProgramVars *vars);
extern void _libc_fork_init(const struct _libc_functions *funcs);
extern void _arc4_init(void);
extern void __atexit_init(void);
extern void __confstr_init(const struct _libc_functions *funcs);
extern void _init_clock_port(void);
extern void __chk_init(void);
extern void __xlocale_init(void);
extern void __guard_setup(const char *apple[]);

void
_libc_initializer(const struct _libc_functions *funcs,
	const char *envp[],
	const char *apple[],
	const struct ProgramVars *vars)
{
	_program_vars_init(vars);
	_libc_fork_init(funcs);
	__confstr_init(funcs);
	__atexit_init();
	_init_clock_port();
	__chk_init();
	__xlocale_init();
	__guard_setup(apple);
}


void
__libc_init(const struct ProgramVars *vars,
	void (*atfork_prepare)(void),
	void (*atfork_parent)(void),
	void (*atfork_child)(void),
	const char *apple[])
{
	const struct _libc_functions funcs = {
		.version = 1,
		.atfork_prepare = atfork_prepare,
		.atfork_parent = atfork_parent,
		.atfork_child = atfork_child,
		.dirhelper = NULL,
	};
	
	return _libc_initializer(&funcs, NULL, apple, vars);
}
