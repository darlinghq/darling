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
 * Relocation types used in the arm implementation.  Relocation entries for
 * things other than instructions use the same generic relocation as discribed
 * in <mach-o/reloc.h> and their r_type is ARM_RELOC_VANILLA, one of the
 * *_SECTDIFF or the *_PB_LA_PTR types.  The rest of the relocation types are
 * for instructions.  Since they are for instructions the r_address field
 * indicates the 32 bit instruction that the relocation is to be preformed on.
 */
enum reloc_type_arm
{
    ARM_RELOC_VANILLA,	/* generic relocation as discribed above */
    ARM_RELOC_PAIR,	/* the second relocation entry of a pair */
    ARM_RELOC_SECTDIFF,	/* a PAIR follows with subtract symbol value */
    ARM_RELOC_LOCAL_SECTDIFF, /* like ARM_RELOC_SECTDIFF, but the symbol
				 referenced was local.  */
    ARM_RELOC_PB_LA_PTR,/* prebound lazy pointer */
    ARM_RELOC_BR24,	/* 24 bit branch displacement (to a word address) */
    ARM_THUMB_RELOC_BR22, /* 22 bit branch displacement (to a half-word
			     address) */
    ARM_THUMB_32BIT_BRANCH, /* obsolete - a thumb 32-bit branch instruction
			     possibly needing page-spanning branch workaround */

    /*
     * For these two r_type relocations they always have a pair following them
     * and the r_length bits are used differently.  The encoding of the
     * r_length is as follows:
     * low bit of r_length:
     *  0 - :lower16: for movw instructions
     *  1 - :upper16: for movt instructions
     * high bit of r_length:
     *  0 - arm instructions
     *  1 - thumb instructions   
     * the other half of the relocated expression is in the following pair
     * relocation entry in the the low 16 bits of r_address field.
     */
    ARM_RELOC_HALF,
    ARM_RELOC_HALF_SECTDIFF
};
