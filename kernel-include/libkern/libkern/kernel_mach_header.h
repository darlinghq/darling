/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
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
 *	File: libkern/kernel_mach_header.h
 *
 *	Definitions for accessing mach-o headers.
 *
 * NOTE:	These functions work on Mach-O headers compatible with
 *		the currently running kernel, and cannot be used against mach
 *		headers other than that of the currently running kernel.
 *
 */

#ifndef _KERNEL_MACH_HEADER_
#define _KERNEL_MACH_HEADER_

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/mach_types.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>

#if     !KERNEL
#error this header for kernel use only
#endif

#if defined(__LP64__)

typedef struct mach_header_64   kernel_mach_header_t;
typedef struct segment_command_64 kernel_segment_command_t;
typedef struct section_64               kernel_section_t;
typedef struct nlist_64         kernel_nlist_t;

#define MH_MAGIC_KERNEL         MH_MAGIC_64
#define LC_SEGMENT_KERNEL       LC_SEGMENT_64

#else

typedef struct mach_header              kernel_mach_header_t;
typedef struct segment_command  kernel_segment_command_t;
typedef struct section                  kernel_section_t;
typedef struct nlist            kernel_nlist_t;

#define MH_MAGIC_KERNEL         MH_MAGIC
#define LC_SEGMENT_KERNEL               LC_SEGMENT
#define SECT_CONSTRUCTOR                "__constructor"
#define SECT_DESTRUCTOR                 "__destructor"

#endif

#define SECT_MODINITFUNC                "__mod_init_func"
#define SECT_MODTERMFUNC                "__mod_term_func"

extern kernel_mach_header_t _mh_execute_header;

vm_offset_t getlastaddr(void);

kernel_segment_command_t *firstseg(void);
kernel_segment_command_t *firstsegfromheader(kernel_mach_header_t *header);
kernel_segment_command_t *nextsegfromheader(
	kernel_mach_header_t    *header,
	kernel_segment_command_t        *seg);
kernel_segment_command_t *getsegbyname(const char *seg_name);
kernel_segment_command_t *getsegbynamefromheader(
	kernel_mach_header_t    *header,
	const char              *seg_name);
void *getsegdatafromheader(kernel_mach_header_t *, const char *, unsigned long *);
kernel_section_t *getsectbyname(const char *seg_name, const char *sect_name);
kernel_section_t *getsectbynamefromheader(
	kernel_mach_header_t    *header,
	const char              *seg_name,
	const char              *sect_name);
uint32_t getsectoffsetfromheader(
	kernel_mach_header_t *mhp,
	const char *segname,
	const char *sectname);
void *getsectdatafromheader(kernel_mach_header_t *, const char *, const char *, unsigned long *);
kernel_section_t *firstsect(kernel_segment_command_t *sgp);
kernel_section_t *nextsect(kernel_segment_command_t *sgp, kernel_section_t *sp);
void *getcommandfromheader(kernel_mach_header_t *, uint32_t);
void *getuuidfromheader(kernel_mach_header_t *, unsigned long *);

#ifdef __cplusplus
}
#endif

#endif  /* _KERNEL_MACH_HEADER_ */
