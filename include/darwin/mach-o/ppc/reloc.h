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
 * Relocation types used in the ppc implementation.  Relocation entries for
 * things other than instructions use the same generic relocation as discribed
 * above and their r_type is RELOC_VANILLA.  The rest of the relocation types
 * are for instructions.  Since they are for instructions the r_address field
 * indicates the 32 bit instruction that the relocation is to be preformed on.
 * The fields r_pcrel and r_length are ignored for non-RELOC_VANILLA r_types
 * except for PPC_RELOC_BR14.
 *
 * For PPC_RELOC_BR14 if the r_length is the unused value 3, then the branch was
 * statically predicted setting or clearing the Y-bit based on the sign of the
 * displacement or the opcode.  If this is the case the static linker must flip
 * the value of the Y-bit if the sign of the displacement changes for non-branch
 * always conditions.
 */
enum reloc_type_ppc
{
    PPC_RELOC_VANILLA,	/* generic relocation as discribed above */
    PPC_RELOC_PAIR,	/* the second relocation entry of a pair */
    PPC_RELOC_BR14,	/* 14 bit branch displacement (to a word address) */
    PPC_RELOC_BR24,	/* 24 bit branch displacement (to a word address) */
    PPC_RELOC_HI16,	/* a PAIR follows with the low half */
    PPC_RELOC_LO16,	/* a PAIR follows with the high half */
    PPC_RELOC_HA16,	/* Same as the RELOC_HI16 except the low 16 bits and the
			 * high 16 bits are added together with the low 16 bits
			 * sign extened first.  This means if bit 15 of the low
			 * 16 bits is set the high 16 bits stored in the
			 * instruction will be adjusted.
			 */
    PPC_RELOC_LO14,	/* Same as the LO16 except that the low 2 bits are not
			 * stored in the instruction and are always zero.  This
			 * is used in double word load/store instructions.
			 */
    PPC_RELOC_SECTDIFF,	/* a PAIR follows with subtract symbol value */
    PPC_RELOC_PB_LA_PTR,/* prebound lazy pointer */
    PPC_RELOC_HI16_SECTDIFF, /* section difference forms of above.  a PAIR */
    PPC_RELOC_LO16_SECTDIFF, /* follows these with subtract symbol value */
    PPC_RELOC_HA16_SECTDIFF,
    PPC_RELOC_JBSR,
    PPC_RELOC_LO14_SECTDIFF,
    PPC_RELOC_LOCAL_SECTDIFF  /* like PPC_RELOC_SECTDIFF, but the symbol
				 referenced was local.  */
};
