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
 * Intel386 Family:	Task State Segment.
 *
 * HISTORY
 *
 * 29 March 1992 ? at NeXT
 *	Created.
 */

#ifndef _ARCH_I386_TSS_H_
#define _ARCH_I386_TSS_H_

#include <architecture/i386/sel.h>

/*
 * Task State segment.
 */

typedef struct tss {
    sel_t		oldtss;
    unsigned int		:0;
    unsigned int	esp0;
    sel_t		ss0;
    unsigned int		:0;
    unsigned int	esp1;
    sel_t		ss1;
    unsigned int		:0;
    unsigned int	esp2;
    sel_t		ss2;
    unsigned int		:0;
    unsigned int	cr3;
    unsigned int	eip;
    unsigned int	eflags;
    unsigned int	eax;
    unsigned int	ecx;
    unsigned int	edx;
    unsigned int	ebx;
    unsigned int	esp;
    unsigned int	ebp;
    unsigned int	esi;
    unsigned int	edi;
    sel_t		es;
    unsigned int		:0;
    sel_t		cs;
    unsigned int		:0;
    sel_t		ss;
    unsigned int		:0;
    sel_t		ds;
    unsigned int		:0;
    sel_t		fs;
    unsigned int		:0;
    sel_t		gs;
    unsigned int		:0;
    sel_t		ldt;
    unsigned int		:0;
    unsigned int	t	:1,
    				:15,
			io_bmap	:16;
} tss_t;

#define TSS_SIZE(n)	(sizeof (struct tss) + (n))

/*
 * Task State segment descriptor.
 */

typedef struct tss_desc {
    unsigned short	limit00;
    unsigned short	base00;
    unsigned char	base16;
    unsigned char	type	:5,
#define DESC_TSS	0x09
			dpl	:2,
			present	:1;
    unsigned char	limit16	:4,
				:3,
			granular:1;
    unsigned char	base24;
} tss_desc_t;

/*
 * Task gate descriptor.
 */
 
typedef struct task_gate {
    unsigned short		:16;
    sel_t		tss;
    unsigned int		:8,
    			type	:5,
#define DESC_TASK_GATE	0x05
			dpl	:2,
			present	:1,
				:0;
} task_gate_t;

#endif	/* _ARCH_I386_TSS_H_ */
