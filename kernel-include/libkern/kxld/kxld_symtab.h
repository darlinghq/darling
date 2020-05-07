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
#ifndef _KXLD_SYMTAB_H_
#define _KXLD_SYMTAB_H_

#include <sys/types.h>
#if KERNEL
    #include <libkern/kxld_types.h>
#else
    #include "kxld_types.h"
#endif

#include "kxld_sym.h"
#include "kxld_seg.h"

struct kxld_array;
struct symtab_command;
typedef struct kxld_symtab KXLDSymtab;
typedef struct kxld_symtab_iterator KXLDSymtabIterator;

struct kxld_symtab_iterator {
	const KXLDSymtab *symtab;
	u_int idx;
	KXLDSymPredicateTest test;
	boolean_t negate;
};

/*******************************************************************************
* Constructors and Destructors
*******************************************************************************/

size_t kxld_symtab_sizeof(void)
__attribute__((const, visibility("hidden")));

#if KXLD_USER_OR_ILP32
kern_return_t kxld_symtab_init_from_macho_32(KXLDSymtab *symtab,
    struct symtab_command *src, u_char *macho, KXLDSeg * kernel_linkedit_seg)
__attribute__((nonnull(1, 2), visibility("hidden")));
#endif /* KXLD_USER_OR_ILP32 */

#if KXLD_USER_OR_LP64
kern_return_t kxld_symtab_init_from_macho_64(KXLDSymtab *symtab,
    struct symtab_command *src, u_char *macho, KXLDSeg * kernel_linkedit_seg)
__attribute__((nonnull(1, 2), visibility("hidden")));
#endif /* KXLD_USER_OR_ILP64 */

void kxld_symtab_iterator_init(KXLDSymtabIterator *iter,
    const KXLDSymtab *symtab, KXLDSymPredicateTest test, boolean_t negate)
__attribute__((nonnull, visibility("hidden")));

void kxld_symtab_clear(KXLDSymtab *symtab)
__attribute__((nonnull, visibility("hidden")));

void kxld_symtab_deinit(KXLDSymtab *symtab)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Accessors
*******************************************************************************/

u_int kxld_symtab_get_num_symbols(const KXLDSymtab *symtab)
__attribute__((pure, nonnull, visibility("hidden")));

KXLDSym * kxld_symtab_get_symbol_by_index(const KXLDSymtab *symtab, u_int idx)
__attribute__((pure, nonnull, visibility("hidden")));

KXLDSym * kxld_symtab_get_symbol_by_name(const KXLDSymtab *symtab,
    const char *name)
__attribute__((pure, nonnull, visibility("hidden")));

KXLDSym * kxld_symtab_get_locally_defined_symbol_by_name(
	const KXLDSymtab *symtab, const char *name)
__attribute__((pure, nonnull, visibility("hidden")));

KXLDSym * kxld_symtab_get_cxx_symbol_by_value(const KXLDSymtab *symtab,
    kxld_addr_t value)
__attribute__((pure, nonnull, visibility("hidden")));

kern_return_t kxld_symtab_get_sym_index(const KXLDSymtab *symtab,
    const KXLDSym * sym, u_int *idx)
__attribute__((nonnull, visibility("hidden")));

u_long kxld_symtab_get_macho_header_size(void)
__attribute__((pure, visibility("hidden")));

u_long kxld_symtab_get_macho_data_size(const KXLDSymtab *symtab,
    boolean_t is_32_bit)
__attribute__((pure, nonnull, visibility("hidden")));

kern_return_t
kxld_symtab_export_macho(const KXLDSymtab *symtab, u_char *buf,
    u_long *header_offset, u_long header_size,
    u_long *data_offset, u_long data_size,
    boolean_t is_32_bit)
__attribute__((nonnull, visibility("hidden")));

u_int kxld_symtab_iterator_get_num_remaining(const KXLDSymtabIterator *iter)
__attribute__((pure, nonnull, visibility("hidden")));

/*******************************************************************************
* Modifiers
*******************************************************************************/

kern_return_t kxld_symtab_index_symbols_by_name(KXLDSymtab *symtab)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_symtab_index_cxx_symbols_by_value(KXLDSymtab *symtab)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_symtab_relocate(KXLDSymtab *symtab,
    const struct kxld_array *sectarray)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_symtab_add_symbol(KXLDSymtab *symtab, char *name,
    kxld_addr_t link_addr, KXLDSym **symout)
__attribute__((nonnull, visibility("hidden")));

KXLDSym * kxld_symtab_iterator_get_next(KXLDSymtabIterator *iter)
__attribute__((nonnull, visibility("hidden")));

void kxld_symtab_iterator_reset(KXLDSymtabIterator *iter)
__attribute__((nonnull, visibility("hidden")));

#endif /* _KXLD_SYMTAB_H_ */
