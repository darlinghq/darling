/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
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
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
#ifdef	PRIVATE

#ifndef	_MACH_I386_SYSCALL_SW_H_
#define _MACH_I386_SYSCALL_SW_H_

#include <architecture/i386/asm_help.h>

/*
 * Software interrupt codes for 32-bit system call entry:
 */
#define UNIX_INT     0x80
#define MACH_INT     0x81
#define MACHDEP_INT  0x82
#define DIAG_INT     0x83

#if defined(__i386__)

#ifndef KERNEL
/*
 * Syscall entry macros for use in libc:
 */
#define UNIX_SYSCALL_TRAP	\
	int $(UNIX_INT)
#define MACHDEP_SYSCALL_TRAP	\
	int $(MACHDEP_INT)

/*
 * Macro to generate Mach call stubs in libc:
 */

#define kernel_trap(trap_name,trap_number,number_args) \
LEAF(_##trap_name,0) ;\
	movl	$##trap_number, %eax	;\
	call	__sysenter_trap		;\
END(_##trap_name)

#endif /* !KERNEL */

#endif /* defined(__i386__) */

#if defined(__x86_64__)

#ifndef KERNEL

#define UNIX_SYSCALL_TRAP	\
	syscall
#define MACHDEP_SYSCALL_TRAP	\
	syscall

/*
 * Macro to generate Mach call stubs in Libc.
 * Existing calls use negative numbers for Mach traps, so
 * until we change those and change the 32-bit kernel_trap
 * macro above, we negate those numbers here for the 64-bit
 * code path.
 */
#define kernel_trap(trap_name,trap_number,number_args) \
LEAF(_##trap_name,0) ;\
	movq	%rcx, %r10	;\
	movl	$ SYSCALL_CONSTRUCT_MACH(-##trap_number), %eax	;\
	syscall		;\
END(_##trap_name)

#endif /* !KERNEL */

#endif /* defined(__x86_64__) */

/*
 * Syscall classes for 64-bit system call entry.
 * For 64-bit users, the 32-bit syscall number is partitioned
 * with the high-order bits representing the class and low-order
 * bits being the syscall number within that class.
 * The high-order 32-bits of the 64-bit syscall number are unused.
 * All system classes enter the kernel via the syscall instruction.
 *
 * These are not #ifdef'd for x86-64 because they might be used for
 * 32-bit someday and so the 64-bit comm page in a 32-bit kernel
 * can use them.
 */
#define SYSCALL_CLASS_SHIFT	24
#define SYSCALL_CLASS_MASK	(0xFF << SYSCALL_CLASS_SHIFT)
#define SYSCALL_NUMBER_MASK	(~SYSCALL_CLASS_MASK)

#define	I386_SYSCALL_CLASS_MASK		SYSCALL_CLASS_MASK
#define	I386_SYSCALL_ARG_BYTES_SHIFT	(16)
#define	I386_SYSCALL_ARG_DWORDS_SHIFT	(I386_SYSCALL_ARG_BYTES_SHIFT + 2)
#define	I386_SYSCALL_ARG_BYTES_NUM	(64) /* Must be <= sizeof(uu_arg) */
#define	I386_SYSCALL_ARG_DWORDS_MASK	((I386_SYSCALL_ARG_BYTES_NUM >> 2) -1)
#define	I386_SYSCALL_ARG_BYTES_MASK	(((I386_SYSCALL_ARG_BYTES_NUM -1)&~0x3) << I386_SYSCALL_ARG_BYTES_SHIFT)
#define	I386_SYSCALL_NUMBER_MASK	(0xFFFF)

#define SYSCALL_CLASS_NONE	0	/* Invalid */
#define SYSCALL_CLASS_MACH	1	/* Mach */	
#define SYSCALL_CLASS_UNIX	2	/* Unix/BSD */
#define SYSCALL_CLASS_MDEP	3	/* Machine-dependent */
#define SYSCALL_CLASS_DIAG	4	/* Diagnostics */
#define SYSCALL_CLASS_IPC	5	/* Mach IPC */

/* Macros to simpllfy constructing syscall numbers. */
#define SYSCALL_CONSTRUCT_MACH(syscall_number) \
			((SYSCALL_CLASS_MACH << SYSCALL_CLASS_SHIFT) | \
			 (SYSCALL_NUMBER_MASK & (syscall_number)))
#define SYSCALL_CONSTRUCT_UNIX(syscall_number) \
			((SYSCALL_CLASS_UNIX << SYSCALL_CLASS_SHIFT) | \
			 (SYSCALL_NUMBER_MASK & (syscall_number)))
#define SYSCALL_CONSTRUCT_MDEP(syscall_number) \
			((SYSCALL_CLASS_MDEP << SYSCALL_CLASS_SHIFT) | \
			 (SYSCALL_NUMBER_MASK & (syscall_number)))
#define SYSCALL_CONSTRUCT_DIAG(syscall_number) \
			((SYSCALL_CLASS_DIAG << SYSCALL_CLASS_SHIFT) | \
			 (SYSCALL_NUMBER_MASK & (syscall_number)))

#endif	/* _MACH_I386_SYSCALL_SW_H_ */

#endif	/* PRIVATE */
