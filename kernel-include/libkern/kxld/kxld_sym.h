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
#ifndef _KXLD_SYMBOL_H_
#define _KXLD_SYMBOL_H_

#include <sys/types.h>
#if KERNEL
    #include <libkern/kxld_types.h>
#else
    #include "kxld_types.h"
#endif

struct kxld_sect;
struct nlist;
struct nlist_64;
typedef struct kxld_sym KXLDSym;
typedef boolean_t (*KXLDSymPredicateTest)(const KXLDSym *sym);

struct kxld_sym {
	char *name;                   // The symbol's name
	char *alias;                  // The indirect symbol's alias name
	kxld_addr_t base_addr;        // The symbol's base address
	kxld_addr_t link_addr;        // The relocated address
	kxld_addr_t got_addr;         // The address of this symbol's GOT entry
	uint16_t desc;
	uint8_t type;
	uint8_t sectnum;              // The symbol's section number
	uint8_t relocated_sectnum;
	u_int is_absolute:1,          // Set for absolute symbols
	    is_section:1,             // Set for section symbols
	    is_undefined:1,           // Set for undefined symbols
	    is_indirect:1,            // Set for indirect symbols
	    is_common:1,              // Set for common symbols
	    is_external:1,            // Set for external symbols
	    is_stab:1,                // Set for stab symbols
	    is_weak:1,                // Set for weak definition symbols
	    is_resolved:1,            // For symbols that have been resolved
	                              // externally and should not be exported
	    is_obsolete:1,            // For symbols marked as obsolete
	    is_replaced:1,            // Set for symbols replaced by patching
	    is_got:1,                 // Has an entry in the GOT
	    is_cxx:1,                 // Set for C++ symbols
	    is_pure_virtual:1,        // Set for pure virtual symbols
	    is_class_vtable:1,        // Set for vtable symbols of classes
	    is_meta_vtable:1,         // Set for vtable symbols of MetaClasses
	    is_padslot:1,             // Set for pad slot symbols
	    is_metaclass:1,           // Set for metaclass symbols
	    is_super_metaclass_pointer:1, // Set for super metaclass pointer syms
	    is_thumb:1;               // Set for thumb symbols (ARM only)
};

/*******************************************************************************
* Constructors and destructors
*******************************************************************************/

#if KXLD_USER_OR_ILP32
kern_return_t kxld_sym_init_from_macho32(KXLDSym *sym, char *strtab,
    const struct nlist *src) __attribute__((nonnull, visibility("hidden")));
#endif

#if KXLD_USER_OR_LP64
kern_return_t kxld_sym_init_from_macho64(KXLDSym *sym, char *strtab,
    const struct nlist_64 *src) __attribute__((nonnull, visibility("hidden")));
#endif

void kxld_sym_init_absolute(KXLDSym *sym, char *name, kxld_addr_t link_addr)
__attribute__((nonnull, visibility("hidden")));

void kxld_sym_deinit(KXLDSym *sym)
__attribute__((nonnull, visibility("hidden")));

void kxld_sym_destroy(KXLDSym *sym)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Accessors
*******************************************************************************/

boolean_t kxld_sym_is_absolute(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_section(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_defined(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_defined_locally(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_external(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_exported(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_undefined(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_indirect(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_replaced(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

/* We don't wrap this in KXLD_USER_OR_COMMON because even though common symbols
 * aren't always supported, we always need to be able to detect them.
 */
boolean_t kxld_sym_is_common(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_unresolved(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_obsolete(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

#if KXLD_USER_OR_GOT
boolean_t kxld_sym_is_got(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_GOT */

boolean_t kxld_sym_is_stab(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_weak(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_cxx(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_pure_virtual(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_vtable(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_class_vtable(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_metaclass_vtable(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_padslot(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_metaclass(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_is_super_metaclass_pointer(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_name_is_pure_virtual(const char *name)
__attribute__((pure, nonnull, visibility("hidden")));

boolean_t kxld_sym_name_is_padslot(const char *name)
__attribute__((pure, nonnull, visibility("hidden")));

u_int kxld_sym_get_section_offset(const KXLDSym *sym,
    const struct kxld_sect *sect)
__attribute__((pure, nonnull, visibility("hidden")));

#if KXLD_USER_OR_COMMON
kxld_size_t kxld_sym_get_common_size(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));

u_int kxld_sym_get_common_align(const KXLDSym *sym)
__attribute__((pure, nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_COMMON */

kern_return_t kxld_sym_get_class_name_from_metaclass(const KXLDSym *sym,
    char class_name[], u_long class_name_len)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_sym_get_class_name_from_super_metaclass_pointer(
	const KXLDSym *sym, char class_name[], u_long class_name_len)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_sym_get_class_name_from_vtable(const KXLDSym *sym,
    char class_name[], u_long class_name_len)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_sym_get_class_name_from_vtable_name(const char *vtable_name,
    char class_name[], u_long class_name_len)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_sym_get_vtable_name_from_class_name(const char *class_name,
    char vtable_name[], u_long vtable_name_len)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_sym_get_meta_vtable_name_from_class_name(const char *class_name,
    char meta_vtable_name[], u_long meta_vtable_name_len)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_sym_get_final_sym_name_from_class_name(const char *class_name,
    char final_sym_name[], u_long final_sym_name_len)
__attribute__((nonnull, visibility("hidden")));

u_long kxld_sym_get_function_prefix_from_class_name(const char *class_name,
    char function_prefix[], u_long function_prefix_len)
__attribute__((nonnull, visibility("hidden")));

#if KXLD_USER_OR_ILP32
kern_return_t kxld_sym_export_macho_32(const KXLDSym *sym, u_char *nl,
    char *strtab, u_long *stroff, u_long strsize)
__attribute__((nonnull, visibility("hidden")));
#endif

#if KXLD_USER_OR_LP64
kern_return_t kxld_sym_export_macho_64(const KXLDSym *sym, u_char *nl,
    char *strtab, u_long *stroff, u_long strsize)
__attribute__((nonnull, visibility("hidden")));
#endif

/*******************************************************************************
* Mutators
*******************************************************************************/

void kxld_sym_relocate(KXLDSym *sym, const struct kxld_sect *sect)
__attribute__((nonnull, visibility("hidden")));

#if KXLD_USER_OR_GOT
void kxld_sym_set_got(KXLDSym *sym)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_GOT */

kern_return_t kxld_sym_resolve(KXLDSym *sym, const kxld_addr_t addr)
__attribute__((nonnull, visibility("hidden")));

#if KXLD_USER_OR_COMMON
kern_return_t kxld_sym_resolve_common(KXLDSym *sym, u_int sectnum,
    kxld_addr_t base_addr)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_COMMON */

void kxld_sym_delete(KXLDSym *sym)
__attribute__((nonnull, visibility("hidden")));

void kxld_sym_patch(KXLDSym *sym)
__attribute__((nonnull, visibility("hidden")));

void kxld_sym_mark_private(KXLDSym *sym)
__attribute__((nonnull, visibility("hidden")));

#endif /* _KXLD_SYMBOL_H_ */
