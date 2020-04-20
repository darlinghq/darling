/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
 * FILE_ID: syscall_sw.h
 */

#ifndef	_MACH_ARM_SYSCALL_SW_H_
#define _MACH_ARM_SYSCALL_SW_H_

#if defined(__arm__)

#include <mach/machine/vm_param.h>
#include <architecture/arm/asm_help.h>

/* 0 to 4 args are already loaded in r0-r3 */
#define _kernel_trap_0to4(trap_name, trap_number) \
	mov		r12, # ## trap_number			/* load syscall number */ ; \
	swi		#SWI_SYSCALL					; \
	bx		lr								/* return */ ;

#define _kernel_trap_5(trap_name, trap_number) \
	mov		ip, sp							/* save pointer to args */ ; \
	stmfd	sp!, { r4-r5 }					/* save r4-r5, keep stack 64-bit aligned */; \
	ldr		r4, [ ip ]						/* load arg 5 */ ; \
	mov		r12, # ## trap_number			/* load syscall number */ ; \
	swi		#SWI_SYSCALL					; \
	ldmfd	sp!, { r4-r5 }					/* restore r4-r5 */ ;\
	bx		lr								/* return */ ;

#define _kernel_trap_6to9(trap_name, trap_number, save_regs, arg_regs) \
	mov		ip, sp							/* save pointer to args */ ; \
	stmfd	sp!, { save_regs }				/* callee saved regs */; \
	ldmia	ip, { arg_regs }				/* load arg registers (above r0-r3) */ ;\
	mov		r12, # ## trap_number			/* load syscall number */ ; \
	swi		#SWI_SYSCALL					; \
	ldmfd	sp!, { save_regs }	 			/* restore callee saved regs */ ;\
	bx		lr								/* return */ ;

#define COMMA ,

/* For the armv7k ABI, the alignment requirements may add padding. So we
 * let the kernel figure it out and push extra on the stack to avoid un-needed
 * copy-ins. We are relying on arguments that aren't in registers starting 
 * 32 bytes from sp.  */
#if __BIGGEST_ALIGNMENT__ > 4

#define _kernel_trap_0(trap_name, trap_number) _kernel_trap_0to4(trap_name, trap_number)
#define _kernel_trap_1(trap_name, trap_number) _kernel_trap_0to4(trap_name, trap_number)
#define _kernel_trap_2(trap_name, trap_number) _kernel_trap_0to4(trap_name, trap_number)
#define _kernel_trap_3(trap_name, trap_number) _kernel_trap_0to4(trap_name, trap_number)
#define _kernel_trap_4(trap_name, trap_number) _kernel_trap_6to9(trap_name, trap_number, r4-r5, r4-r5)
#undef _kernel_trap_5
#define _kernel_trap_5(trap_name, trap_number) _kernel_trap_6to9(trap_name, trap_number, r4-r5, r4-r5)
#define _kernel_trap_6(trap_name, trap_number) _kernel_trap_6to9(trap_name, trap_number, r4-r6 COMMA r8, r4-r6 COMMA r8)
#define _kernel_trap_7(trap_name, trap_number) _kernel_trap_6to9(trap_name, trap_number, r4-r6 COMMA r8, r4-r6 COMMA r8)
#define _kernel_trap_8(trap_name, trap_number) _kernel_trap_6to9(trap_name, trap_number, r4-r6 COMMA r8, r4-r6 COMMA r8)
#define _kernel_trap_9(trap_name, trap_number) _kernel_trap_6to9(trap_name, trap_number, r4-r6 COMMA r8, r4-r6 COMMA r8)
#else // !(__BIGGEST_ALIGNMENT__ > 4)

#define _kernel_trap_0(trap_name, trap_number) _kernel_trap_0to4(trap_name, trap_number)
#define _kernel_trap_1(trap_name, trap_number) _kernel_trap_0to4(trap_name, trap_number)
#define _kernel_trap_2(trap_name, trap_number) _kernel_trap_0to4(trap_name, trap_number)
#define _kernel_trap_3(trap_name, trap_number) _kernel_trap_0to4(trap_name, trap_number)
#define _kernel_trap_4(trap_name, trap_number) _kernel_trap_0to4(trap_name, trap_number)
/* _kernel_trap_5 defined above */
#define _kernel_trap_6(trap_name, trap_number) _kernel_trap_6to9(trap_name, trap_number, r4-r5, r4-r5)
/* need to save r8 not just for alignment but because mach_msg_trap overwrites the eighth argument */
#define _kernel_trap_7(trap_name, trap_number) _kernel_trap_6to9(trap_name, trap_number, r4-r6 COMMA r8, r4-r6)
#define _kernel_trap_8(trap_name, trap_number) _kernel_trap_6to9(trap_name, trap_number, r4-r6 COMMA r8, r4-r6 COMMA r8)
/* there is only one nine-argument trap (mach_msg_overwrite_trap) and it doesn't use the ninth argument */
#define _kernel_trap_9(trap_name, trap_number) _kernel_trap_8(trap_name, trap_number)

#endif // __BIGGEST_ALIGNMENT__ > 4


/* select the appropriate trap macro based off the number of args */
#define kernel_trap(trap_name, trap_number, num_args) \
	LEAF(_##trap_name, 0)	\
	_kernel_trap_##num_args(trap_name, trap_number)

#elif defined(__arm64__)

#include <mach/machine/vm_param.h>

#define kernel_trap(trap_name, trap_number, num_args) \
.globl _##trap_name                                           %% \
.text                                                         %% \
.align  2                                                     %% \
_##trap_name:                                                 %% \
    mov x16, #(trap_number)                                   %% \
    svc #SWI_SYSCALL                                          %% \
    ret

#else
#error Unsupported architecture
#endif

#endif	/* _MACH_ARM_SYSCALL_SW_H_ */
