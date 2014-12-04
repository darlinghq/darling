/*
 * Copyright (c) 2007, 2011 Apple Inc. All rights reserved.
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

#include <sys/syscall.h>

#define UC_TRAD			1
#define UC_FLAVOR		30

/* Structure fields for ucontext and mcontext.  */
#define UCONTEXT_UC_MCONTEXT	48

#define MCONTEXT_ES_EXCEPTION	0
#define MCONTEXT_SS_RAX		16
#define MCONTEXT_SS_RBX		24
#define MCONTEXT_SS_RCX		32
#define MCONTEXT_SS_RDX		40
#define MCONTEXT_SS_RDI		48
#define MCONTEXT_SS_RSI		56
#define MCONTEXT_SS_RBP		64
#define MCONTEXT_SS_RSP		72
#define MCONTEXT_SS_R8		80
#define MCONTEXT_SS_RIP		144

/* register use:
	%rbx	uctx
	
void
_sigtramp(
	union __sigaction_u __sigaction_u,	%rdi
	int 			sigstyle,	%rsi
	int 			sig,		%rdx
	siginfo_t		*sinfo,		%rcx
	ucontext_t		*uctx		%r8
)
*/

	.globl __sigtramp
	.text
	.align 4,0x90
__sigtramp:
Lstart:
	/* Although this routine does not need any stack frame, various parts
	   of the OS can't analyse the stack without them.  */
	pushq	%rbp
	movq	%rsp, %rbp

	movq	%rdi, %rax	# set up address for call

#if defined(__DYNAMIC__)
	incl	___in_sigtramp(%rip)
#endif
	/* Save uctx in %rbx.  */
	movq	%r8, %rbx
	/* Call the signal handler.
	   Some variants are not supposed to get the last two parameters,
	   but the test to prevent this is more expensive than just passing
	   them.  */
	movl	%edx, %edi
	movq	%rcx, %rsi
	movq	%r8, %rdx
Lcall_start:
	call	*%rax
Lcall_end:
#if defined(__DYNAMIC__)
	decl	___in_sigtramp(%rip)
#endif
	movq	%rbx, %rdi
	movl	$ UC_FLAVOR, %esi
	callq	___sigreturn
	ret
Lend:

/* DWARF unwind table #defines.  */
#define DW_CFA_advance_loc_4 0x44
#define DW_CFA_def_cfa 0x0c
#define DW_CFA_def_cfa_expression 0x0F
#define DW_CFA_expression 0x10
#define DW_CFA_val_expression 0x16
#define DW_CFA_offset(column) 0x80+(column)

/* DWARF expression #defines.  */
#define DW_OP_deref 0x06
#define DW_OP_const1u 0x08
#define DW_OP_dup 0x12
#define DW_OP_drop 0x13
#define DW_OP_over 0x14
#define DW_OP_pick 0x15
#define DW_OP_swap 0x16
#define DW_OP_rot 0x17
#define DW_OP_abs 0x19
#define DW_OP_and 0x1a
#define DW_OP_div 0x1b
#define DW_OP_minus 0x1c
#define DW_OP_mod 0x1d
#define DW_OP_mul 0x1e
#define DW_OP_neg 0x1f
#define DW_OP_not 0x20
#define DW_OP_or 0x21
#define DW_OP_plus 0x22
#define DW_OP_plus_uconst 0x23
#define DW_OP_shl 0x24
#define DW_OP_shr 0x25
#define DW_OP_shra 0x26
#define DW_OP_xor 0x27
#define DW_OP_skip 0x2f
#define DW_OP_bra 0x28
#define DW_OP_eq 0x29
#define DW_OP_ge 0x2A
#define DW_OP_gt 0x2B
#define DW_OP_le 0x2C
#define DW_OP_lt 0x2D
#define DW_OP_ne 0x2E
#define DW_OP_lit(n) 0x30+(n)
#define DW_OP_breg(n) 0x70+(n)
#define DW_OP_deref_size 0x94

/* The location expression we'll use.  */

#define loc_expr_for_reg(regno, offs)				\
	.byte DW_CFA_expression, regno, 5 /* block length */,	\
	 DW_OP_breg(3), UCONTEXT_UC_MCONTEXT, DW_OP_deref,	\
	 DW_OP_plus_uconst, offs

/* For r8 through r13 */
#define loc_expr_rN(regno) \
  loc_expr_for_reg(regno, MCONTEXT_SS_R8+(8*(regno-8)))

/* For r14 through r15 */
#define loc_expr_rN_long(regno)						\
	.byte DW_CFA_expression, regno, 6 /* block length */,		\
	 DW_OP_breg(3), UCONTEXT_UC_MCONTEXT, DW_OP_deref,		\
	 DW_OP_plus_uconst, MCONTEXT_SS_R8+(8*(regno-8)), 1

	/* Unwind tables.  */
	.section __TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
EH_frame1:
	.set L$set$0,LECIE1-LSCIE1
	.long L$set$0	# Length of Common Information Entry
LSCIE1:
	.long	0	# CIE Identifier Tag
	.byte	0x1	# CIE Version
	.ascii "zRS\0"	# CIE Augmentation
	.byte	0x1	# uleb128 0x1; CIE Code Alignment Factor
	.byte	0x78	# sleb128 -8; CIE Data Alignment Factor
	.byte	0x10	# CIE RA Column
	.byte	0x1	# uleb128 0x1; Augmentation size
	.byte	0x10	# FDE Encoding (pcrel)
	.byte	DW_CFA_def_cfa
	.byte	0x7	# uleb128 0x5
	.byte	0x8	# uleb128 0x4
	.byte	DW_CFA_offset(16)
	.byte	0x1	# uleb128 0x1
	.byte	DW_CFA_offset(16)	// duplicate DW_CFA_offset (rip, -8) tells linker to not make compact unwind
	.byte	0x1	# uleb128 0x1
	.align 3
LECIE1:
	.globl _sigtramp.eh
_sigtramp.eh:
LSFDE1:
	.set L$set$1,LEFDE1-LASFDE1
	.long L$set$1	# FDE Length
LASFDE1:
	.long	LASFDE1-EH_frame1	# FDE CIE offset
	.quad	Lstart-.	# FDE initial location
	.set L$set$2,Lend-Lstart
	.quad	L$set$2	# FDE address range
	.byte	0x0	# uleb128 0x0; Augmentation size

	/* Now for the expressions, which all compute
	   uctx->uc_mcontext->register
	   for each register.
	   
	   Describe even the registers that are not call-saved because they
	   might be being used in the prologue to save other registers.
	   Only integer registers are described at present.    */

	loc_expr_for_reg (0, MCONTEXT_SS_RAX)
	loc_expr_for_reg (1, MCONTEXT_SS_RDX)
	loc_expr_for_reg (2, MCONTEXT_SS_RCX)
	loc_expr_for_reg (3, MCONTEXT_SS_RBX)
	loc_expr_for_reg (4, MCONTEXT_SS_RSI)
	loc_expr_for_reg (5, MCONTEXT_SS_RDI)
	loc_expr_for_reg (6, MCONTEXT_SS_RBP)
	loc_expr_for_reg (7, MCONTEXT_SS_RSP)
	loc_expr_rN (8)
	loc_expr_rN (9)
	loc_expr_rN (10)
	loc_expr_rN (11)
	loc_expr_rN (12)
	loc_expr_rN (13)
	loc_expr_rN_long (14)
	loc_expr_rN_long (15)

	/* The Intel architecture classifies exceptions into three categories,
	   'faults' which put the address of the faulting instruction
	   in EIP, 'traps' which put the following instruction in EIP,
	   and 'aborts' which don't typically report the instruction
	   causing the exception.

	   The traps are #BP and #OF.  */

	.byte DW_CFA_val_expression, 16
	.set L$set$3,Lpc_end-Lpc_start
	.byte L$set$3
Lpc_start:
	/* Push the mcontext address twice.  */
	.byte DW_OP_breg(3), UCONTEXT_UC_MCONTEXT, DW_OP_deref, DW_OP_dup
	/* Find the value of EIP.  */
	.byte   DW_OP_plus_uconst, MCONTEXT_SS_RIP, MCONTEXT_SS_RIP >> 7
	.byte   DW_OP_deref, DW_OP_swap
	/* Determine the exception type.  */
	.byte   DW_OP_plus_uconst, MCONTEXT_ES_EXCEPTION, DW_OP_deref_size, 4
	/* Check whether it is #BP (3) or #OF (4).  */
	.byte   DW_OP_dup, DW_OP_lit(3), DW_OP_ne
	.byte    DW_OP_swap, DW_OP_lit(4), DW_OP_ne, DW_OP_and
	/* If it is not, then add 1 to the instruction address, so as to point
	   within or past the faulting instruction.  */
	.byte   DW_OP_plus
Lpc_end:

	/* The CFA will have been saved as the value of RSP (it is not
	   RSP+8).  */
	.byte DW_CFA_def_cfa_expression
	.set L$set$4,Lcfa_end-Lcfa_start
	.byte L$set$4
Lcfa_start:
	.byte DW_OP_breg(3), UCONTEXT_UC_MCONTEXT, DW_OP_deref
	.byte  DW_OP_plus_uconst, MCONTEXT_SS_RSP, DW_OP_deref
Lcfa_end:

	.align 3
LEFDE1:
		
	.subsections_via_symbols
