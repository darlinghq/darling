/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
/* Copyright (c) 1996 NeXT Software, Inc.  All rights reserved.
 *
 *	File:	architecture/ppc/pseudo_inst.h
 *	Author:	Mike DeMoney
 *
 *	This header file defines assembler pseudo-instruction macros for
 *	for the ppc.
 *
 *	NOTE: This is obviously only useful to include in assembly
 *	code source.
 *
 *	ALSO NOTE: These macros don't attempt to be 64-bit compatable
 *
 * HISTORY
 * 29-Dec-96  Umesh Vaishampayan  (umeshv@NeXT.com)
 *	Ported from m98k.
 * 05-Nov-92  Mike DeMoney (mike@next.com)
 *	Created.
 */

#ifndef	_ARCH_PPC_PSEUDO_INST_H_
#define	_ARCH_PPC_PSEUDO_INST_H_

#include <architecture/ppc/reg_help.h>
#include <architecture/ppc/asm_help.h>

#ifdef	__ASSEMBLER__

/*
 * Pseudo instruction definitions
 */

/*
 * Macro package initialization
 */
	.set	__no_at,0		/* allow at by default */

/*
 * .at_off -- disable use of at by macros
 * .at_on -- enable use of at by macros
 */
.macro	.at_off
	.set	__no_at,1
.endmacro

.macro	.at_on
	.set	__no_at,0
.endmacro

/*
 * li32	rD,IMMED
 *
 * Load 32-bit immediate into rD
 * FIXME: Need a way to undefine built-in macro for this.
 */
.macro	li32				// li32	rD,immed
.if	$n != 2
	.abort	"invalid operands of li32"
.endif
.abs	__is_abs,$1
.if	!__is_abs
	addis	$0,0,hi16($1)
	ori	$0,$0,lo16($1)
.elseif	$1 == 0
	addi	$0,0,0
.elseif	($1 & 0xffff) == 0
	addis	$0,0,hi16($1)
.elseif	($1 & 0xffff8000) == 0
	addi	$0,0,$1
.elseif ($1 & 0xffff8000) == 0xffff8000
	addi	$0,0,$1
.else
	addis	$0,0,hi16($1)
	ori	$0,$0,lo16($1)
.endif
.endmacro
 

/*
 * andi32. rD,rS1,IMMED
 *
 * Perform "andi." with (possibly) 32-bit immediate
 */
.macro	andi32.				// andi32. rD,rS1,IMMED
.if	$n != 3
	.abort	"invalid operands of andi."
.endif
	.set	__used_at,0
.abs	__is_abs,$2
.if	!__is_abs
	.set	__used_at,1
	li32	at,$2
	and.	$0,$1,at
.elseif	($2 & 0xffff0000) == 0
	andi.	$0,$1,$2
.elseif	($2 & 0xffff) == 0
	andis.	$0,$1,hi16($2)
.else
	.set	__used_at,1
	li32	at,$2
	and.	$0,$1,at
.endif
.if	__no_at & __used_at
	.abort	"Macro uses at while .no_at in effect"
.endif
.endmacro
 
/*
 * ori32	rD,rS1,IMMED
 *
 * Perform "ori" with (possibly) 32-bit immediate
 */
.macro	ori32				// ori32	rD,rS1,IMMED
.if	$n != 3
	.abort	"invalid operands of ori"
.endif
.abs	__is_abs,$2
.if	!__is_abs
	oris	$0,$1,hi16($2)
	ori	$0,$1,lo16($2)
.elseif	($2 & 0xffff0000) == 0
	ori	$0,$1,$2
.elseif	($2 & 0xffff) == 0
	oris	$0,$1,hi16($2)
.else
	oris	$0,$1,hi16($2)
	ori	$0,$1,lo16($2)
.endif
.endmacro
 
/*
 * xori32	rD,rS1,IMMED
 *
 * Perform "xor" with (possibly) 32-bit immediate
 */
.macro	xori32				// xori32	rD,rS1,IMMED
.if	$n != 3
	.abort	"invalid operands of xori"
.endif
.abs	__is_abs,$2
.if	!__is_abs
	xoris	$0,$1,hi16($2)
	xori	$0,$1,lo16($2)
.elseif	($2 & 0xffff0000) == 0
	xori	$0,$1,$2
.elseif	($2 & 0xffff) == 0
	xoris	$0,$1,hi16($2)
.else
	xoris	$0,$1,hi16($2)
	xori	$0,$1,lo16($2)
.endif
.endmacro


/*
 * MEMREF_INST -- macros to memory referencing instructions
 * "capable" of dealing with 32 bit offsets.
 *
 * NOTE: Because the assembler doesn't have any mechanism for easily
 * parsing the d(rS) syntax of register-displacement form instructions,
 * these instructions do NOT mirror the normal memory reference
 * instructions.  The following "transformation" is used:
 *	lbz	rD,d(rS)
 * becomes:
 *	lbz32	rD,rS,d
 * I.e.: "32" is appended to the instruction name and the base register
 * and displacement become the 2'nd and 3'rd comma-separated operands.
 *
 * The forms:
 *	lbz32	rD,d
 * and:
 *	lbz32	rD,rS
 * are also recognized and the missing operand is assumed 0.
 *
 * ALSO NOTE: r0 or zt should never be used as rS in these instructions.
 * Use "0" as rS in this case.
 */
#define	MEMREF_INST(op)						\
.macro	op ## 32						@\
.set	__used_at,0						@\
.if	$n == 3							@\
 .greg	__is_greg,$1						@\
 .abs	__is_abs,$2						@\
 .if	__is_abs						@\
  .if	($2 & 0xffff8000) == 0					@\
	op	$0,$2($1)					@\
  .elseif ($2 & 0xffff8000) == 0xffff8000			@\
	op	$0,$2($1)					@\
  .else								@\
   .if	!__is_greg						@\
	.set	__used_at,1					@\
	lis	at,ha16($2)					@\
	op	$0,lo16($2)(at)					@\
   .else							@\
  	.set	__used_at,1					@\
	lis	at,ha16($2)					@\
	add	at,at,$1					@\
	op	$0,lo16($2)(at)					@\
   .endif							@\
  .endif							@\
 .else								@\
  .if	!__is_greg						@\
	.set	__used_at,1					@\
	lis	at,ha16($2)					@\
	op	$0,lo16($2)(at)					@\
  .else								@\
  	.set	__used_at,1					@\
	lis	at,ha16($2)					@\
	add	at,at,$1					@\
	op	$0,lo16($2)(at)					@\
  .endif							@\
 .endif								@\
.elseif	$n == 2							@\
 .greg	__is_greg,$1						@\
 .if	!__is_greg						@\
  .abs	__is_abs,$1						@\
  .if	__is_abs						@\
   .if	($1 & 0xffff8000) == 0					@\
	op	$0,$1(0)					@\
   .elseif ($1 & 0xffff8000) == 0xffff8000			@\
	op	$0,$1(0)					@\
   .else							@\
	.set	__used_at,1					@\
	lis	at,ha16($1)					@\
	op	$0,lo16($1)(at)					@\
   .endif							@\
  .else								@\
	.set	__used_at,1					@\
	lis	at,ha16($1)					@\
	op	$0,lo16($1)(at)					@\
  .endif							@\
 .else								@\
	op	$0,0($1)					@\
 .endif								@\
.else								@\
	.abort "Invalid operands of " #op "32"			@\
.endif								@\
.if	__no_at &  __used_at					@\
	.abort	"Macro uses at while .no_at in effect"		@\
.endif								@\
.endmacro

MEMREF_INST(lbz)
MEMREF_INST(lhz)
MEMREF_INST(lha)
MEMREF_INST(lwz)
MEMREF_INST(lwa)
MEMREF_INST(ld)

MEMREF_INST(stb)
MEMREF_INST(sth)
MEMREF_INST(stw)
MEMREF_INST(std)

MEMREF_INST(lmw)
MEMREF_INST(lmd)
MEMREF_INST(stmw)
MEMREF_INST(stmd)

/*
 * ARITH_INST -- define 32-bit immediate forms of arithmetic
 * instructions
 *
 *	E.g.	addi32	rD,rS,IMMED
 */
#define	ARITH_INST(op, op3, sf)					\
.macro	op ## 32 ## sf						@\
.if	$n != 3							@\
	.abort	"invalid operands to " #op "32"			@\
.endif								@\
.abs	__is_abs,$2						@\
.if	__is_abs						@\
 .if	($2 & 0xffff8000) == 0					@\
	op##sf	$0,$1,$2					@\
 .elseif	($2 & 0xffff8000) == 0xffff8000			@\
	op##sf	$0,$1,$2					@\
 .elseif	__no_at						@\
	.abort	"Macro uses at while .no_at in effect"		@\
 .else								@\
	li32	at,$2						@\
	op3##sf	$0,$1,at					@\
 .endif								@\
.elseif	__no_at							@\
	.abort	"Macro uses at while .no_at in effect"		@\
.else								@\
	li32	at,$2						@\
	op3##sf	$0,$1,at					@\
.endif								@\
.endmacro

ARITH_INST(addi, add, )
ARITH_INST(subi, sub, )
ARITH_INST(addic, addc, )
ARITH_INST(subic, subc, )
ARITH_INST(addic, addc, .)
ARITH_INST(subic, subc, .)
ARITH_INST(mulli, mull, )

/*
 * CMPEX_INST -- define 32-bit immediate forms of extended compare
 * instructions
 *
 *	E.g.	cmpwi32	cr3,rS,IMMED
 *		cmpwi32	rS,IMMED
 */
#define	CMPEX_INST(op, op3)					\
.macro	op ## 32						@\
.if	$n == 3							@\
 .abs	__is_abs,$2						@\
 .if	__is_abs						@\
  .if	($2 & 0xffff8000) == 0					@\
	op	$0,$1,$2					@\
  .elseif	($2 & 0xffff8000) == 0xffff8000			@\
	op	$0,$1,$2					@\
  .elseif	__no_at						@\
	.abort	"Macro uses at while .no_at in effect"		@\
  .else								@\
	li32	at,$2						@\
	op3	$0,$1,at					@\
  .endif							@\
 .elseif	__no_at						@\
	.abort	"Macro uses at while .no_at in effect"		@\
 .else								@\
	li32	at,$2						@\
	op3	$0,$1,at					@\
 .endif								@\
.elseif	$n == 2							@\
 .abs	__is_abs,$1						@\
 .if	__is_abs						@\
  .if	($1 & 0xffff8000) == 0					@\
	op	$0,$1						@\
  .elseif	($1 & 0xffff8000) == 0xffff8000			@\
	op	$0,$1						@\
  .elseif	__no_at						@\
	.abort	"Macro uses at while .no_at in effect"		@\
  .else								@\
	li32	at,$1						@\
	op3	$0,at						@\
  .endif							@\
 .elseif	__no_at						@\
	.abort	"Macro uses at while .no_at in effect"		@\
 .else								@\
	li32	at,$1						@\
	op3	$0,at						@\
 .endif								@\
.else								@\
	.abort	"invalid operands to " #op "32"			@\
.endif								@\
.endmacro

CMPEX_INST(cmpdi, cmpd)
CMPEX_INST(cmpwi, cmpw)
CMPEX_INST(cmpldi, cmpld)
CMPEX_INST(cmplwi, cmplw)

/*
 * CMP_INST -- define 32-bit immediate forms of standard compare
 * instructions
 *
 *	E.g.	cmpi32	cr3,0,rS,IMMED
 */
#define	CMP_INST(op, op3)					\
.macro	op ## 32						@\
.if	$n == 4							@\
 .abs	__is_abs,$3						@\
 .if	__is_abs						@\
  .if	($3 & 0xffff8000) == 0					@\
	op	$0,$1,$2,$3					@\
  .elseif	($3 & 0xffff8000) == 0xffff8000			@\
	op	$0,$1,$2,$3					@\
  .elseif	__no_at						@\
	.abort	"Macro uses at while .no_at in effect"		@\
  .else								@\
	li32	at,$3						@\
	op3	$0,$1,$2,at					@\
  .endif							@\
 .elseif	__no_at						@\
	.abort	"Macro uses at while .no_at in effect"		@\
 .else								@\
	li32	at,$3						@\
	op3	$0,$1,$2,at					@\
 .endif								@\
.else								@\
	.abort	"invalid operands to " #op "32"			@\
.endif								@\
.endmacro

CMP_INST(cmpi, cmp)
CMP_INST(cmpli, cmpl)

#endif	/* __ASSEMBLER__ */

#endif	/* _ARCH_PPC_PSEUDO_INST_H_ */
