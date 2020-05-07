/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#ifndef _KXLD_SEG_H_
#define _KXLD_SEG_H_

#include <mach/vm_prot.h>
#include <sys/types.h>
#if KERNEL
    #include <libkern/kxld_types.h>
#else
    #include "kxld_types.h"
#endif

#include "kxld_array.h"

struct kxld_sect;
struct kxld_symtab;
struct segment_command;
struct segment_command_64;
typedef struct kxld_seg KXLDSeg;

struct kxld_seg {
	char segname[16];
	kxld_addr_t base_addr;
	kxld_addr_t link_addr;
	kxld_size_t vmsize;
	kxld_size_t fileoff;
	KXLDArray sects;
	u_int flags;
	vm_prot_t maxprot;
	vm_prot_t initprot;
};

/*******************************************************************************
* Constructors and Destructors
*******************************************************************************/

#if KXLD_USER_OR_ILP32
kern_return_t kxld_seg_init_from_macho_32(KXLDSeg *seg, struct segment_command *src)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_ILP32 */

#if KXLD_USER_OR_LP64
kern_return_t kxld_seg_init_from_macho_64(KXLDSeg *seg, struct segment_command_64 *src)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_LP64 */

#if KXLD_USER_OR_OBJECT
kern_return_t kxld_seg_create_seg_from_sections(KXLDArray *segarray,
    KXLDArray *sectarray)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_seg_finalize_object_segment(KXLDArray *segarray,
    KXLDArray *section_order, u_long hdrsize)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_seg_init_linkedit(KXLDArray *segs)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_OBJECT */

void kxld_seg_clear(KXLDSeg *seg)
__attribute__((nonnull, visibility("hidden")));

void kxld_seg_deinit(KXLDSeg *seg)
__attribute__((nonnull, visibility("hidden")));


/*******************************************************************************
* Accessors
*******************************************************************************/

kxld_size_t kxld_seg_get_vmsize(const KXLDSeg *seg)
__attribute__((pure, nonnull, visibility("hidden")));

u_long kxld_seg_get_macho_header_size(const KXLDSeg *seg, boolean_t is_32_bit)
__attribute__((pure, nonnull, visibility("hidden")));

#if 0
/* This is no longer used, but may be useful some day... */
u_long kxld_seg_get_macho_data_size(const KXLDSeg *seg)
__attribute__((pure, nonnull, visibility("hidden")));
#endif

kern_return_t
kxld_seg_export_macho_to_file_buffer(const KXLDSeg *seg, u_char *buf,
    u_long *header_offset, u_long header_size,
    u_long *data_offset, u_long data_size,
    boolean_t is_32_bit)
__attribute__((nonnull, visibility("hidden")));

kern_return_t
kxld_seg_export_macho_to_vm(const KXLDSeg *seg,
    u_char *buf,
    u_long *header_offset,
    u_long header_size,
    u_long data_size,
    kxld_addr_t file_link_addr,
    boolean_t is_32_bit)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Modifiers
*******************************************************************************/

kern_return_t kxld_seg_add_section(KXLDSeg *seg, struct kxld_sect *sect)
__attribute__((nonnull, visibility("hidden")));

/* To be called after all sections are added */
kern_return_t kxld_seg_finish_init(KXLDSeg *seg)
__attribute__((nonnull, visibility("hidden")));

void kxld_seg_set_vm_protections(KXLDSeg *seg, boolean_t strict_protections)
__attribute__((nonnull, visibility("hidden")));

void kxld_seg_relocate(KXLDSeg *seg, kxld_addr_t link_addr)
__attribute__((nonnull, visibility("hidden")));

void kxld_seg_populate_linkedit(KXLDSeg *seg, const struct kxld_symtab *symtab,
    boolean_t is_32_bit
#if KXLD_PIC_KEXTS
    , const struct kxld_array *locrelocs
    , const struct kxld_array *extrelocs
    , boolean_t target_supports_slideable_kexts
#endif  /* KXLD_PIC_KEXTS */
    , uint32_t splitinfolc_size
    )
__attribute__((nonnull, visibility("hidden")));

boolean_t kxld_seg_is_split_seg(const KXLDSeg *seg)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_seg_is_text_seg(const KXLDSeg *seg)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_seg_is_text_exec_seg(const KXLDSeg *seg)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_seg_is_data_seg(const KXLDSeg *seg)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_seg_is_data_const_seg(const KXLDSeg *seg)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_seg_is_linkedit_seg(const KXLDSeg *seg)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_seg_is_llvm_cov_seg(const KXLDSeg *seg)
__attribute__((pure, nonnull, visibility("hidden")));

#endif /* _KXLD_SEG_H_ */
