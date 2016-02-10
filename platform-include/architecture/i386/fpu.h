/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1992 NeXT Computer, Inc.
 *
 * Intel386 Family:	Floating Point unit.
 *
 * HISTORY
 *
 * 5 October 1992 ? at NeXT
 *	Added names to previously unamed fields in the mantissa.
 *
 * 5 April 1992 ? at NeXT
 *	Created.
 */

#ifndef _ARCH_I386_FPU_H_
#define _ARCH_I386_FPU_H_

/*
 * Data register.
 */

typedef struct fp_data_reg {
    unsigned short		mant;
    unsigned short		mant1	:16,
				mant2	:16,
				mant3	:16;
    unsigned short		exp	:15,
			    	sign	:1;
} fp_data_reg_t;

/*
 * Data register stack.
 */

typedef struct fp_stack {
    fp_data_reg_t		ST[8];
} fp_stack_t;

/*
 * Register stack tag word.
 */

typedef struct fp_tag {
    unsigned short		tag0	:2,
    				tag1	:2,
				tag2	:2,
				tag3	:2,
    				tag4	:2,
				tag5	:2,
				tag6	:2,
				tag7	:2;
#define FP_TAG_VALID		0
#define FP_TAG_ZERO		1
#define FP_TAG_SPEC		2
#define FP_TAG_EMPTY		3
} fp_tag_t;

/*
 * Status word.
 */

typedef struct fp_status {
    unsigned short		invalid	:1,
    				denorm	:1,
				zdiv	:1,
				ovrfl	:1,
				undfl	:1,
				precis	:1,
				stkflt	:1,
				errsumm	:1,
				c0	:1,
				c1	:1,
				c2	:1,
				tos	:3,
				c3	:1,
				busy	:1;
} fp_status_t;

/*
 * Control word.
 */

typedef struct fp_control {
    unsigned short		invalid	:1,
    				denorm	:1,
				zdiv	:1,
				ovrfl	:1,
				undfl	:1,
				precis	:1,
					:2,
				pc	:2,
#define FP_PREC_24B		0
#define	FP_PREC_53B		2
#define FP_PREC_64B		3
				rc	:2,
#define FP_RND_NEAR		0
#define FP_RND_DOWN		1
#define FP_RND_UP		2
#define FP_CHOP			3
				/*inf*/	:1,
					:3;
} fp_control_t;

#include <architecture/i386/sel.h>

/*
 * Floating point 'environment'
 * used by FSTENV/FLDENV instructions.
 */

typedef struct fp_env {
    fp_control_t		control;
    unsigned short			:16;
    fp_status_t			status;
    unsigned short			:16;
    fp_tag_t			tag;
    unsigned short			:16;
    unsigned int		ip;
    sel_t			cs;
    unsigned short		opcode;
    unsigned int		dp;
    sel_t			ds;
    unsigned short			:16;
} fp_env_t;

/*
 * Floating point state
 * used by FSAVE/FRSTOR instructions.
 */
 
/*
 * To allow the the common idiom of:
 *     #define environ (*_NSGetEnviron())
 * to be used these fields were renamed.  Old code that that does
 * not use this idiom can use the old field names by defining
 * _ARCHITECTURE_I386_FPU_FPSTATE_LEGACY_FIELD_NAMES_ .
 */
typedef struct fp_state {
#if _ARCHITECTURE_I386_FPU_FPSTATE_LEGACY_FIELD_NAMES_
    fp_env_t			environ;
    fp_stack_t			stack;
#else
    fp_env_t			fp_environ;
    fp_stack_t			fp_stack;
#endif
} fp_state_t;

#endif	/* _ARCH_I386_FPU_H_ */
