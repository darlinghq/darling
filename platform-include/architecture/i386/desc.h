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
 * Intel386 Family:	Segment descriptors.
 *
 * HISTORY
 *
 * 29 March 1992 ? at NeXT
 *	Created.
 */

#ifndef _ARCH_I386_DESC_H_
#define _ARCH_I386_DESC_H_

/*
 * Code segment descriptor.
 */

typedef struct code_desc {
    unsigned short	limit00;
    unsigned short	base00;
    unsigned char	base16;
    unsigned char	type	:5,
#define DESC_CODE_EXEC	0x18
#define DESC_CODE_READ	0x1a
			dpl	:2,
			present	:1;
    unsigned char	limit16	:4,
				:2,
			opsz	:1,
#define DESC_CODE_16B	0
#define DESC_CODE_32B	1
			granular:1;
#define DESC_GRAN_BYTE	0
#define DESC_GRAN_PAGE	1
    unsigned char	base24;
} code_desc_t;
 
/*
 * Data segment descriptor.
 */

typedef struct data_desc {
    unsigned short	limit00;
    unsigned short	base00;
    unsigned char	base16;
    unsigned char	type	:5,
#define DESC_DATA_RONLY	0x10
#define DESC_DATA_WRITE	0x12
			dpl	:2,
			present	:1;
    unsigned char	limit16	:4,
				:2,
			stksz	:1,
#define DESC_DATA_16B	0
#define DESC_DATA_32B	1
			granular:1;
    unsigned char	base24;
} data_desc_t;

/*
 * LDT segment descriptor.
 */

typedef struct ldt_desc {
    unsigned short	limit00;
    unsigned short	base00;
    unsigned char	base16;
    unsigned char	type	:5,
#define DESC_LDT	0x02
				:2,
			present	:1;
    unsigned char	limit16	:4,
				:3,
			granular:1;
    unsigned char	base24;
} ldt_desc_t;

#include <architecture/i386/sel.h>

/*
 * Call gate descriptor.
 */
 
typedef struct call_gate {
    unsigned short	offset00;
    sel_t		seg;
    unsigned int	argcnt	:5,
    				:3,
			type	:5,
#define DESC_CALL_GATE	0x0c
			dpl	:2,
			present	:1,
			offset16:16;
} call_gate_t;

/*
 * Trap gate descriptor.
 */
 
typedef struct trap_gate {
    unsigned short	offset00;
    sel_t		seg;
    unsigned int		:8,
    			type	:5,
#define DESC_TRAP_GATE	0x0f
			dpl	:2,
			present	:1,
			offset16:16;
} trap_gate_t;


/*
 * Interrupt gate descriptor.
 */
 
typedef struct intr_gate {
    unsigned short	offset00;
    sel_t		seg;
    unsigned int		:8,
    			type	:5,
#define DESC_INTR_GATE	0x0e
			dpl	:2,
			present	:1,
			offset16:16;
} intr_gate_t;

#endif	/* _ARCH_I386_DESC_H_ */
