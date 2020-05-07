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
/*
 * Copyright (c) 1992 NeXT Computer, Inc.
 *
 * Intel386 Family:	Descriptor tables.
 *
 * HISTORY
 *
 * 30 March 1992 ? at NeXT
 *	Created.
 */

#include <architecture/i386/desc.h>
#include <architecture/i386/tss.h>

/*
 * A totally generic descriptor
 * table entry.
 */

typedef union dt_entry {
    code_desc_t		code;
    data_desc_t		data;
    ldt_desc_t		ldt;
    tss_desc_t		task_state;
    call_gate_t		call_gate;
    trap_gate_t		trap_gate;
    intr_gate_t		intr_gate;
    task_gate_t		task_gate;
} dt_entry_t;

#define DESC_TBL_MAX	8192

/*
 * Global descriptor table.
 */

typedef union gdt_entry {
    code_desc_t		code;
    data_desc_t		data;
    ldt_desc_t		ldt;
    call_gate_t		call_gate;
    task_gate_t		task_gate;
    tss_desc_t		task_state;
} gdt_entry_t;

typedef gdt_entry_t	gdt_t;

/*
 * Interrupt descriptor table.
 */

typedef union idt_entry {
    trap_gate_t		trap_gate;
    intr_gate_t		intr_gate;
    task_gate_t		task_gate;
} idt_entry_t;

typedef idt_entry_t	idt_t;

/*
 * Local descriptor table.
 */

typedef union ldt_entry {
    code_desc_t		code;
    data_desc_t		data;
    call_gate_t		call_gate;
    task_gate_t		task_gate;
} ldt_entry_t;

typedef ldt_entry_t	ldt_t;
