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
#include <platfunc.h>
#include <mach/i386/syscall_sw.h>
	

PLATFUNC_FUNCTION_START(OSSpinLockTry, up, 32, 4)
PLATFUNC_FUNCTION_START(_spin_lock_try, up, 32, 4)
	movl		4(%esp), %ecx 
	xorl		%eax, %eax
	orl		$-1, %edx
	cmpxchgl	%edx, (%ecx)
	setz		%dl
	movzbl		%dl, %eax
	ret
PLATFUNC_DESCRIPTOR(OSSpinLockTry,up,kUP,0)
PLATFUNC_DESCRIPTOR(_spin_lock_try,up,kUP,0)

PLATFUNC_FUNCTION_START_GENERIC(OSSpinLockTry, mp, 32, 4)
PLATFUNC_FUNCTION_START_GENERIC(_spin_lock_try, mp, 32, 4)
	movl		4(%esp), %ecx 
	xorl		%eax, %eax
	orl		$-1, %edx
	lock
	cmpxchgl	%edx, (%ecx)
	setz		%dl
	movzbl		%dl, %eax
	ret
PLATFUNC_DESCRIPTOR(OSSpinLockTry,mp,0,kUP)
PLATFUNC_DESCRIPTOR(_spin_lock_try,mp,0,kUP)


PLATFUNC_FUNCTION_START(spin_lock, up, 32, 4)
PLATFUNC_FUNCTION_START(OSSpinLockLock, up, 32, 4)
PLATFUNC_FUNCTION_START(_spin_lock, up, 32, 4)
Lspin_lock_up:
	movl		4(%esp), %ecx
	xorl		%eax, %eax
	orl		$-1, %edx
	cmpxchgl	%edx, (%ecx)
	jnz		1f
	ret
1:
	/* failed to get lock so relinquish the processor immediately on UP */
	pushl		$1		/* 1 ms				*/
	pushl		$1		/* SWITCH_OPTION_DEPRESS	*/
	pushl		$0		/* THREAD_NULL			*/
	pushl		$0		/* push dummy stack ret addr    */
	movl		$-61,%eax	/* SYSCALL_THREAD_SWITCH */
	int		$(MACH_INT)
	addl		$16, %esp	/* adjust stack*/
	jmp		Lspin_lock_up
PLATFUNC_DESCRIPTOR(spin_lock,up,kUP,0)
PLATFUNC_DESCRIPTOR(OSSpinLockLock,up,kUP,0)
PLATFUNC_DESCRIPTOR(_spin_lock,up,kUP,0)

PLATFUNC_FUNCTION_START_GENERIC(spin_lock, mp, 32, 4)
PLATFUNC_FUNCTION_START_GENERIC(OSSpinLockLock, mp, 32, 4)
PLATFUNC_FUNCTION_START_GENERIC(_spin_lock, mp, 32, 4)
Lspin_lock_mp:
	movl		4(%esp), %ecx
	xorl		%eax, %eax
0:
	orl		$-1, %edx
	lock
	cmpxchgl	%edx, (%ecx)
	jnz		1f
	ret
1:
	xorl		%eax, %eax
	movl		$(MP_SPIN_TRIES), %edx
2:
	pause	
	cmpl		%eax, (%ecx)
	jz		0b		/* favor success and slow down spin loop */
	decl		%edx
	jnz		2b
	/* failed to get lock after spinning so relinquish  */
	pushl		$1		/* 1 ms				*/
	pushl		$1		/* SWITCH_OPTION_DEPRESS	*/
	pushl		$0		/* THREAD_NULL			*/
	pushl		$0		/* push dummy stack ret addr    */
	movl		$-61,%eax	/* SYSCALL_THREAD_SWITCH */
	int		$(MACH_INT)
	addl		$16, %esp	/* adjust stack*/
	jmp		Lspin_lock_mp
PLATFUNC_DESCRIPTOR(spin_lock,mp,0,kUP)
PLATFUNC_DESCRIPTOR(OSSpinLockLock,mp,0,kUP)
PLATFUNC_DESCRIPTOR(_spin_lock,mp,0,kUP)

	.align	2, 0x90
	.globl	_OSSpinLockUnlock
	.globl	_spin_unlock
	.globl	__spin_unlock
_OSSpinLockUnlock:
_spin_unlock:
__spin_unlock:
	movl	4(%esp), %eax
	movl	$0, (%eax)
	ret

