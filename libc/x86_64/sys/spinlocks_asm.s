/*
 * Copyright (c) 2003-2009 Apple, Inc. All rights reserved.
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

#include <sys/appleapiopts.h>
#include <machine/cpu_capabilities.h>
#include <mach/i386/syscall_sw.h>
#include "platfunc.h"

PLATFUNC_FUNCTION_START(OSSpinLockTry, up, 64, 4)
PLATFUNC_FUNCTION_START(_spin_lock_try, up, 64, 4)
	xorl		%eax, %eax
	orl		$-1, %edx
	cmpxchgl	%edx, (%rdi)
	setz		%dl
	movzbl		%dl, %eax
	ret
PLATFUNC_DESCRIPTOR(OSSpinLockTry,up,kUP,0)
PLATFUNC_DESCRIPTOR(_spin_lock_try,up,kUP,0)


PLATFUNC_FUNCTION_START_GENERIC(OSSpinLockTry, mp, 64, 4)
PLATFUNC_FUNCTION_START_GENERIC(_spin_lock_try, mp, 64, 4)
	xorl		%eax, %eax
	orl		$-1, %edx
	lock
	cmpxchgl	%edx, (%rdi)
	setz		%dl
	movzbl		%dl, %eax
	ret
PLATFUNC_DESCRIPTOR(OSSpinLockTry,mp,0,kUP)
PLATFUNC_DESCRIPTOR(_spin_lock_try,mp,0,kUP)


PLATFUNC_FUNCTION_START(OSSpinLockLock, up, 64, 4)
PLATFUNC_FUNCTION_START(_spin_lock, up, 64, 4)
PLATFUNC_FUNCTION_START(spin_lock, up, 64, 4)
	movq		%rdi,%r8
0:
	xorl		%eax, %eax
	orl		$-1, %edx
	cmpxchgl	%edx, (%r8)
	jnz		1f
	ret
1:
	/* failed to get lock so relinquish the processor immediately on UP */
	xorl		%edi,%edi	/* THREAD_NULL			*/
	movl		$1,%esi		/* SWITCH_OPTION_DEPRESS	*/
	movl		$1,%edx		/* 1 ms				*/
	movl		$(SYSCALL_CONSTRUCT_MACH(61)),%eax	/* 61 = thread_switch */
	syscall
	jmp		0b
PLATFUNC_DESCRIPTOR(OSSpinLockLock,up,kUP,0)
PLATFUNC_DESCRIPTOR(_spin_lock,up,kUP,0)
PLATFUNC_DESCRIPTOR(spin_lock,up,kUP,0)
	
	
PLATFUNC_FUNCTION_START_GENERIC(OSSpinLockLock, mp, 64, 4)
PLATFUNC_FUNCTION_START_GENERIC(_spin_lock, mp, 64, 4)
PLATFUNC_FUNCTION_START_GENERIC(spin_lock, mp, 64, 4)
	movq		%rdi,%r8
0:
	xorl		%eax, %eax
	orl		$-1, %edx
	lock
	cmpxchgl	%edx, (%r8)
	jnz		1f
	ret
1:
	xorl		%eax, %eax
	movl		$(MP_SPIN_TRIES), %edx
2:					/* spin for awhile before relinquish */
	pause	
	cmpl		%eax, (%r8)
	jz		0b
	decl		%edx
	jnz		2b
	/* failed to get lock after spinning so relinquish  */
	xorl		%edi,%edi	/* THREAD_NULL			*/
	movl		$1,%esi		/* SWITCH_OPTION_DEPRESS	*/
	movl		$1,%edx		/* 1 ms				*/
	movl		$(SYSCALL_CONSTRUCT_MACH(61)),%eax	/* 61 = thread_switch */
	syscall
	jmp		0b
PLATFUNC_DESCRIPTOR(OSSpinLockLock,mp,0,kUP)
PLATFUNC_DESCRIPTOR(_spin_lock,mp,0,kUP)
PLATFUNC_DESCRIPTOR(spin_lock,mp,0,kUP)

// void OSSpinLockUnlock( OSSpinLock *lock );
	.align	2, 0x90
	.globl	_OSSpinLockUnlock
	.globl	_spin_unlock
	.globl	__spin_unlock
_OSSpinLockUnlock:
_spin_unlock:
__spin_unlock:
	movl	$0, (%rdi)
	ret

