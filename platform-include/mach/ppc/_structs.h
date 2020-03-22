/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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
/*
 * @OSF_COPYRIGHT@
 */

#ifndef	_MACH_PPC__STRUCTS_H_
#define	_MACH_PPC__STRUCTS_H_

#include <sys/cdefs.h>

/*
 * ppc_thread_state is the structure that is exported to user threads for 
 * use in status/mutate calls.  This structure should never change.
 *
 */

#if __DARWIN_UNIX03
#define	_STRUCT_PPC_THREAD_STATE	struct __darwin_ppc_thread_state
_STRUCT_PPC_THREAD_STATE
{
	unsigned int __srr0;	/* Instruction address register (PC) */
	unsigned int __srr1;	/* Machine state register (supervisor) */
	unsigned int __r0;
	unsigned int __r1;
	unsigned int __r2;
	unsigned int __r3;
	unsigned int __r4;
	unsigned int __r5;
	unsigned int __r6;
	unsigned int __r7;
	unsigned int __r8;
	unsigned int __r9;
	unsigned int __r10;
	unsigned int __r11;
	unsigned int __r12;
	unsigned int __r13;
	unsigned int __r14;
	unsigned int __r15;
	unsigned int __r16;
	unsigned int __r17;
	unsigned int __r18;
	unsigned int __r19;
	unsigned int __r20;
	unsigned int __r21;
	unsigned int __r22;
	unsigned int __r23;
	unsigned int __r24;
	unsigned int __r25;
	unsigned int __r26;
	unsigned int __r27;
	unsigned int __r28;
	unsigned int __r29;
	unsigned int __r30;
	unsigned int __r31;

	unsigned int __cr;	/* Condition register */
	unsigned int __xer;	/* User's integer exception register */
	unsigned int __lr;	/* Link register */
	unsigned int __ctr;	/* Count register */
	unsigned int __mq;	/* MQ register (601 only) */

	unsigned int __vrsave;	/* Vector Save Register */
};
#else /* !__DARWIN_UNIX03 */
#define	_STRUCT_PPC_THREAD_STATE	struct ppc_thread_state
_STRUCT_PPC_THREAD_STATE
{
	unsigned int srr0;	/* Instruction address register (PC) */
	unsigned int srr1;	/* Machine state register (supervisor) */
	unsigned int r0;
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int r12;
	unsigned int r13;
	unsigned int r14;
	unsigned int r15;
	unsigned int r16;
	unsigned int r17;
	unsigned int r18;
	unsigned int r19;
	unsigned int r20;
	unsigned int r21;
	unsigned int r22;
	unsigned int r23;
	unsigned int r24;
	unsigned int r25;
	unsigned int r26;
	unsigned int r27;
	unsigned int r28;
	unsigned int r29;
	unsigned int r30;
	unsigned int r31;

	unsigned int cr;	/* Condition register */
	unsigned int xer;	/* User's integer exception register */
	unsigned int lr;	/* Link register */
	unsigned int ctr;	/* Count register */
	unsigned int mq;	/* MQ register (601 only) */

	unsigned int vrsave;	/* Vector Save Register */
};
#endif /* __DARWIN_UNIX03 */

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)

#pragma pack(4)			/* Make sure the structure stays as we defined it */

#if __DARWIN_UNIX03
#define _STRUCT_PPC_THREAD_STATE64	struct __darwin_ppc_thread_state64
_STRUCT_PPC_THREAD_STATE64
{
	unsigned long long __srr0;	/* Instruction address register (PC) */
	unsigned long long __srr1;	/* Machine state register (supervisor) */
	unsigned long long __r0;
	unsigned long long __r1;
	unsigned long long __r2;
	unsigned long long __r3;
	unsigned long long __r4;
	unsigned long long __r5;
	unsigned long long __r6;
	unsigned long long __r7;
	unsigned long long __r8;
	unsigned long long __r9;
	unsigned long long __r10;
	unsigned long long __r11;
	unsigned long long __r12;
	unsigned long long __r13;
	unsigned long long __r14;
	unsigned long long __r15;
	unsigned long long __r16;
	unsigned long long __r17;
	unsigned long long __r18;
	unsigned long long __r19;
	unsigned long long __r20;
	unsigned long long __r21;
	unsigned long long __r22;
	unsigned long long __r23;
	unsigned long long __r24;
	unsigned long long __r25;
	unsigned long long __r26;
	unsigned long long __r27;
	unsigned long long __r28;
	unsigned long long __r29;
	unsigned long long __r30;
	unsigned long long __r31;

	unsigned int __cr;		/* Condition register */
	unsigned long long __xer;	/* User's integer exception register */
	unsigned long long __lr;	/* Link register */
	unsigned long long __ctr;	/* Count register */

	unsigned int __vrsave;		/* Vector Save Register */
};
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_PPC_THREAD_STATE64	struct ppc_thread_state64
_STRUCT_PPC_THREAD_STATE64
{
	unsigned long long srr0;	/* Instruction address register (PC) */
	unsigned long long srr1;	/* Machine state register (supervisor) */
	unsigned long long r0;
	unsigned long long r1;
	unsigned long long r2;
	unsigned long long r3;
	unsigned long long r4;
	unsigned long long r5;
	unsigned long long r6;
	unsigned long long r7;
	unsigned long long r8;
	unsigned long long r9;
	unsigned long long r10;
	unsigned long long r11;
	unsigned long long r12;
	unsigned long long r13;
	unsigned long long r14;
	unsigned long long r15;
	unsigned long long r16;
	unsigned long long r17;
	unsigned long long r18;
	unsigned long long r19;
	unsigned long long r20;
	unsigned long long r21;
	unsigned long long r22;
	unsigned long long r23;
	unsigned long long r24;
	unsigned long long r25;
	unsigned long long r26;
	unsigned long long r27;
	unsigned long long r28;
	unsigned long long r29;
	unsigned long long r30;
	unsigned long long r31;

	unsigned int cr;		/* Condition register */
	unsigned long long xer;		/* User's integer exception register */
	unsigned long long lr;		/* Link register */
	unsigned long long ctr;		/* Count register */

	unsigned int vrsave;		/* Vector Save Register */
};
#endif /* __DARWIN_UNIX03 */

#pragma pack()

#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

/* This structure should be double-word aligned for performance */

#if __DARWIN_UNIX03
#define	_STRUCT_PPC_FLOAT_STATE	struct __darwin_ppc_float_state
_STRUCT_PPC_FLOAT_STATE
{
	double  __fpregs[32];

	unsigned int __fpscr_pad; /* fpscr is 64 bits, 32 bits of rubbish */
	unsigned int __fpscr;	/* floating point status register */
};
#else /* !__DARWIN_UNIX03 */
#define	_STRUCT_PPC_FLOAT_STATE	struct ppc_float_state
_STRUCT_PPC_FLOAT_STATE
{
	double  fpregs[32];

	unsigned int fpscr_pad; /* fpscr is 64 bits, 32 bits of rubbish */
	unsigned int fpscr;	/* floating point status register */
};
#endif /* __DARWIN_UNIX03 */

#pragma pack(4)		/* Make sure the structure stays as we defined it */

#if __DARWIN_UNIX03
#define _STRUCT_PPC_VECTOR_STATE	struct __darwin_ppc_vector_state
_STRUCT_PPC_VECTOR_STATE
{
#if defined(__LP64__)
	unsigned int	__save_vr[32][4];
	unsigned int	__save_vscr[4];
#else
	unsigned long	__save_vr[32][4];
	unsigned long	__save_vscr[4];
#endif
	unsigned int	__save_pad5[4];
	unsigned int	__save_vrvalid;			/* VRs that have been saved */
	unsigned int	__save_pad6[7];
};
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_PPC_VECTOR_STATE	struct ppc_vector_state
_STRUCT_PPC_VECTOR_STATE
{
#if defined(__LP64__)
	unsigned int	save_vr[32][4];
	unsigned int	save_vscr[4];
#else
	unsigned long	save_vr[32][4];
	unsigned long	save_vscr[4];
#endif
	unsigned int	save_pad5[4];
	unsigned int	save_vrvalid;			/* VRs that have been saved */
	unsigned int	save_pad6[7];
};
#endif /* __DARWIN_UNIX03 */

#pragma pack()

/*
 * ppc_exception_state
 *
 * This structure corresponds to some additional state of the user
 * registers as saved in the PCB upon kernel entry. They are only
 * available if an exception is passed out of the kernel, and even
 * then not all are guaranteed to be updated.
 *
 * Some padding is included in this structure which allows space for
 * servers to store temporary values if need be, to maintain binary
 * compatiblity.
 */

/* Exception state for 32-bit thread (on 32-bit processor) */
/* Still available on 64-bit processors, but may fall short */
/* of covering the full potential state (hi half available). */

#pragma pack(4)	/* Make sure the structure stays as we defined it */

#if __DARWIN_UNIX03
#define _STRUCT_PPC_EXCEPTION_STATE	struct __darwin_ppc_exception_state
_STRUCT_PPC_EXCEPTION_STATE
{
#if defined(__LP64__)
	unsigned int __dar;		/* Fault registers for coredump */
	unsigned int __dsisr;
	unsigned int __exception;	/* number of powerpc exception taken */
	unsigned int __pad0;		/* align to 16 bytes */
	unsigned int __pad1[4];		/* space in PCB "just in case" */
#else
	unsigned long __dar;		/* Fault registers for coredump */
	unsigned long __dsisr;
	unsigned long __exception;	/* number of powerpc exception taken */
	unsigned long __pad0;		/* align to 16 bytes */
	unsigned long __pad1[4];	/* space in PCB "just in case" */
#endif
};
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_PPC_EXCEPTION_STATE	struct ppc_exception_state
_STRUCT_PPC_EXCEPTION_STATE
{
#if defined(__LP64__)
	unsigned int dar;		/* Fault registers for coredump */
	unsigned int dsisr;
	unsigned int exception;		/* number of powerpc exception taken */
	unsigned int pad0;		/* align to 16 bytes */
	unsigned int pad1[4];		/* space in PCB "just in case" */
#else
	unsigned long dar;		/* Fault registers for coredump */
	unsigned long dsisr;
	unsigned long exception;	/* number of powerpc exception taken */
	unsigned long pad0;		/* align to 16 bytes */
	unsigned long pad1[4];		/* space in PCB "just in case" */
#endif
};
#endif /* __DARWIN_UNIX03 */

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#if __DARWIN_UNIX03
#define _STRUCT_PPC_EXCEPTION_STATE64	struct __darwin_ppc_exception_state64
_STRUCT_PPC_EXCEPTION_STATE64
{
	unsigned long long __dar;	/* Fault registers for coredump */
#if defined(__LP64__)
	unsigned int  __dsisr;
	unsigned int  __exception;	/* number of powerpc exception taken */
	unsigned int  __pad1[4];	/* space in PCB "just in case" */
#else
	unsigned long __dsisr;
	unsigned long __exception;	/* number of powerpc exception taken */
	unsigned long __pad1[4];	/* space in PCB "just in case" */
#endif
};
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_PPC_EXCEPTION_STATE64	struct ppc_exception_state64
_STRUCT_PPC_EXCEPTION_STATE64
{
	unsigned long long dar;		/* Fault registers for coredump */
#if defined(__LP64__)
	unsigned int  dsisr;
	unsigned int  exception;	/* number of powerpc exception taken */
	unsigned int  pad1[4];		/* space in PCB "just in case" */
#else
	unsigned long dsisr;
	unsigned long exception;	/* number of powerpc exception taken */
	unsigned long pad1[4];		/* space in PCB "just in case" */
#endif
};
#endif /* __DARWIN_UNIX03 */
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

#pragma pack()

#endif /* _MACH_PPC__STRUCTS_H_ */
