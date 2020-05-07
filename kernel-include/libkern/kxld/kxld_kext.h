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
#ifndef _KXLD_KEXT_H_
#define _KXLD_KEXT_H_

#include <sys/types.h>
#if KERNEL
    #include <libkern/kxld_types.h>
#else
    #include "kxld_types.h"
#endif

struct kxld_array;
struct kxld_kext;
struct kxld_dict;
struct kxld_object;
struct kxld_sect;
struct kxld_seg;
struct kxld_symtab;
struct kxld_vtable;
typedef struct kxld_kext KXLDKext;

/*******************************************************************************
* Constructors and Destructors
*******************************************************************************/

size_t kxld_kext_sizeof(void)
__attribute__((const, visibility("hidden")));

kern_return_t kxld_kext_init(KXLDKext *kext, struct kxld_object *kext_object,
    struct kxld_object *interface_object)
__attribute__((nonnull(1, 2), visibility("hidden")));

void kxld_kext_clear(KXLDKext *kext)
__attribute__((nonnull, visibility("hidden")));

void kxld_kext_deinit(KXLDKext *kext)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Accessors
*******************************************************************************/

kern_return_t kxld_kext_export_symbols(const KXLDKext *kext,
    struct kxld_dict *defined_symbols_by_name,
    struct kxld_dict *obsolete_symbols_by_name,
    struct kxld_dict *defined_cxx_symbols_by_value)
__attribute__((nonnull(1), visibility("hidden")));

void kxld_kext_get_vmsize_for_seg_by_name(const KXLDKext *kext,
    const char *segname,
    u_long *vmsize)
__attribute__((nonnull, visibility("hidden")));

void kxld_kext_get_vmsize(const KXLDKext *kext,
    u_long *header_size, u_long *vmsize)
__attribute__((nonnull, visibility("hidden")));

void kxld_kext_set_linked_object_size(KXLDKext *kext, u_long vmsize)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_kext_export_linked_object(const KXLDKext *kext,
    void *linked_object,
    kxld_addr_t *kmod_info)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Modifiers
*******************************************************************************/
kern_return_t kxld_kext_export_vtables(KXLDKext *kext,
    const struct kxld_dict *defined_cxx_symbols,
    const struct kxld_dict *defined_symbols,
    struct kxld_dict *vtables)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_kext_relocate(KXLDKext *kext,
    kxld_addr_t link_address,
    struct kxld_dict *patched_vtables,
    const struct kxld_dict *defined_symbols,
    const struct kxld_dict *obsolete_symbols,
    const struct kxld_dict *defined_cxx_symbols)
__attribute__((nonnull(1, 3, 4), visibility("hidden")));


#endif /* _KXLD_KEXT_H_ */
