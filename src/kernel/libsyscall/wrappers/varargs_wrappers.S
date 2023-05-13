/*
 * Copyright (c) 2011-2013 Apple Inc. All rights reserved.
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

#ifdef __arm64__

#include "../custom/SYS.h"
#include <mach/arm64/asm.h>

/* 
 * Stubs are to handle the ARM64 ABI for variadic functions' 
 * not matching the ABI used by the system call handler.
 */

/*
 *	sem_t* sem_open(const char *name, int oflag, ...);
 *	sem_t* __sem_open(const char *name, int oflag, int mode, int value);
 */
MI_ENTRY_POINT(_sem_open) 
	ARM64_STACK_PROLOG
	PUSH_FRAME
#if __LP64__
	ldp	x2, x3, [fp, #16]
#else
	ldp	w2, w3, [fp, #16]
#endif
	MI_CALL_EXTERNAL(___sem_open)
#if !__LP64__
	/* xnu returns a 64-bit '-1' on failure, but pointers must have the high
	 * 32-bits set to zero. The following instruction is equivalent to
	 * masking off the top 32-bits.
	 */
	mov w0, w0
#endif
	POP_FRAME
	ARM64_STACK_EPILOG

/* 
 * int shm_open(const char *, int, ...);
 * int __shm_open(const char*, int oflag, int mode);
 */
MI_ENTRY_POINT(_shm_open)
	ARM64_STACK_PROLOG
	PUSH_FRAME
#if __LP64__
	ldr	x2, [fp, #16]
#else
	ldr	w2, [fp, #16]
#endif
	MI_CALL_EXTERNAL(___shm_open)
	POP_FRAME
	ARM64_STACK_EPILOG

/*
 * int msgsys(int, ...);
 * int __msgsys(int which, int a2, int a3, int a4, int a5);
 */
MI_ENTRY_POINT(_msgsys)
	ARM64_STACK_PROLOG
	PUSH_FRAME
#if __LP64__
	ldp	x1, x2, [fp, #16]
	ldp	x3, x4, [fp, #32]
#else
	ldp	w1, w2, [fp, #16]
	ldp	w3, w4, [fp, #24]
#endif
	MI_CALL_EXTERNAL(___msgsys)
	POP_FRAME

/*
 * int semsys(int, ...);
 * int __semsys(int which, int a2, int a3, int a4, int a5);
 */
MI_ENTRY_POINT(_semsys)
	ARM64_STACK_PROLOG
	PUSH_FRAME
#if __LP64__
	ldp	x1, x2, [fp, #16]
	ldp	x3, x4, [fp, #32]
#else
	ldp	w1, w2, [fp, #16]
	ldp	w3, w4, [fp, #24]
#endif
	MI_CALL_EXTERNAL(___semsys)
	POP_FRAME
	ARM64_STACK_EPILOG

/* 
 * int	semctl(int, int, int, ...);
 * int __semctl(int semid, int semnum, int cmd, semun_t arg);
 */
 MI_ENTRY_POINT(_semctl)
	ARM64_STACK_PROLOG
	PUSH_FRAME
#if __LP64__
	ldr	x3, [fp, #16]
#else
	ldr	w3, [fp, #16]
#endif
	MI_CALL_EXTERNAL(___semctl)
	POP_FRAME
	ARM64_STACK_EPILOG

/* 
 * int	shmsys(int, ...);
 * int __shmsys(int which, int a2, int a3, int a4);
 */
 MI_ENTRY_POINT(_shmsys)
	ARM64_STACK_PROLOG
	PUSH_FRAME
#if __LP64__
	ldp	x1, x2, [fp, #16]
	ldr	x3, [fp, #32]
#else
	ldp	w1, w2, [fp, #16]
	ldr	w3, [fp, #24]
#endif
	MI_CALL_EXTERNAL(___shmsys)
	POP_FRAME
	ARM64_STACK_EPILOG

#endif /* defined(__arm64__) */
