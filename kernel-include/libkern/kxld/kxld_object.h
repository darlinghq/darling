/*
 * Copyright (c) 2009, 2012 Apple Inc. All rights reserved.
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
#ifndef _KXLD_OBJECT_H_
#define _KXLD_OBJECT_H_

#include <sys/types.h>
#if KERNEL
    #include <libkern/kxld_types.h>
#else
    #include "kxld_types.h"
#endif

struct kxld_array;
struct kxld_dict;
struct kxld_reloc;
struct kxld_relocator;
struct kxld_sect;
struct kxld_sym;
struct kxld_symtab;

typedef struct kxld_object KXLDObject;

/*******************************************************************************
* Constructors and destructors
*******************************************************************************/

size_t kxld_object_sizeof(void)
__attribute__((const, visibility("hidden")));

kern_return_t kxld_object_init_from_macho(KXLDObject *object,
    u_char *file, u_long size, const char *name,
    struct kxld_array *section_order,
    cpu_type_t cputype, cpu_subtype_t cpusubtype, KXLDFlags flags)
__attribute__((nonnull(1, 2, 4), visibility("hidden")));

void kxld_object_clear(KXLDObject *object)
__attribute__((nonnull, visibility("hidden")));

void kxld_object_deinit(KXLDObject *object)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Accessors
*******************************************************************************/

const u_char * kxld_object_get_file(const KXLDObject *object)
__attribute__((pure, nonnull, visibility("hidden")));

const char * kxld_object_get_name(const KXLDObject *object)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_object_is_32_bit(const KXLDObject *object)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_object_is_final_image(const KXLDObject *object)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_object_is_kernel(const KXLDObject *object)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_object_is_linked(const KXLDObject *object)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_object_target_supports_strict_patching(const KXLDObject *object)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_object_target_supports_common_symbols(const KXLDObject *object)
__attribute__((pure, nonnull, visibility("hidden")));

const struct kxld_relocator * kxld_object_get_relocator(
	const KXLDObject * object)
__attribute__((pure, nonnull, visibility("hidden")));

const struct kxld_reloc * kxld_object_get_reloc_at_symbol(
	const KXLDObject *object, const struct kxld_sym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

const struct kxld_sym * kxld_object_get_symbol_of_reloc(
	const KXLDObject *object, const struct kxld_reloc *reloc,
	const struct kxld_sect *sect)
__attribute__((pure, nonnull, visibility("hidden")));

const struct kxld_sect * kxld_object_get_section_by_index(
	const KXLDObject *object, u_int sectnum)
__attribute__((pure, nonnull, visibility("hidden")));

const struct kxld_array * kxld_object_get_extrelocs(
	const KXLDObject *object)
__attribute__((pure, nonnull, visibility("hidden")));

const struct kxld_symtab * kxld_object_get_symtab(const KXLDObject *object)
__attribute__((pure, nonnull, visibility("hidden")));

void kxld_object_get_vmsize(const KXLDObject *object, u_long *header_size,
    u_long *vmsize)
__attribute__((nonnull, visibility("hidden")));

void kxld_object_set_linked_object_size(KXLDObject *object, u_long vmsize)
__attribute__((nonnull, visibility("hidden")));

void kxld_object_get_vmsize_for_seg_by_name(const KXLDObject *object,
    const char *segname,
    u_long *vmsize)
__attribute__((nonnull, visibility("hidden")));

splitKextLinkInfo * kxld_object_get_link_info(KXLDObject *object)
__attribute__((nonnull, visibility("hidden")));

void kxld_object_set_link_info(KXLDObject *object,
    splitKextLinkInfo *link_info)
__attribute__((nonnull, visibility("hidden")));


/* This will be the same size as kxld_kext_get_vmsize */
kern_return_t kxld_object_export_linked_object(const KXLDObject *object,
    void *linked_object
    )
__attribute__((nonnull, visibility("hidden")));


/*******************************************************************************
* Modifiers
*******************************************************************************/

kern_return_t kxld_object_index_symbols_by_name(KXLDObject *object)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_object_index_cxx_symbols_by_value(KXLDObject *object)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_object_relocate(KXLDObject *object, kxld_addr_t link_address)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_object_resolve_symbol(KXLDObject *object,
    const struct kxld_sym *sym, kxld_addr_t addr)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_object_patch_symbol(KXLDObject *object,
    const struct kxld_sym *sym)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_object_add_symbol(KXLDObject *object, char *name,
    kxld_addr_t link_addr, const struct kxld_sym **sym_out)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_object_process_relocations(KXLDObject *object,
    const struct kxld_dict *patched_vtables)
__attribute__((nonnull, visibility("hidden")));

#endif /* _KXLD_OBJECT_H_ */
