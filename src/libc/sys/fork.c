/*
 * Copyright (c) 2010 Apple Computer, Inc. All rights reserved.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

extern pid_t __fork(void);
extern void _cthread_fork_prepare();
extern void _cthread_fork_parent();
extern void _cthread_fork_child();

static void (*_libSystem_atfork_prepare)(void) = 0;
static void (*_libSystem_atfork_parent)(void) = 0;
static void (*_libSystem_atfork_child)(void) = 0;

__private_extern__ void _libc_fork_init(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
	_libSystem_atfork_prepare = prepare;
	_libSystem_atfork_parent = parent;
	_libSystem_atfork_child = child;
}

/*
 * fork stub
 */
pid_t
fork(void)
{
	int ret;
	
	_libSystem_atfork_prepare();
	// Reader beware: this __fork() call is yet another wrapper around the actual syscall
	// and lives inside libsyscall. The fork syscall needs some cuddling by asm before it's
	// allowed to see the big wide C world.
	ret = __fork();
	if (-1 == ret)
	{
		// __fork already set errno for us
		_libSystem_atfork_parent();
		return ret;
	}
	
	if (0 == ret)
	{
		// We're the child in this part.
		_libSystem_atfork_child();
		return 0;
	}
	
	_libSystem_atfork_parent();
	return ret;
}

