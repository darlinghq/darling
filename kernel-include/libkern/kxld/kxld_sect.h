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
#ifndef _KXLD_SECT_H_
#define _KXLD_SECT_H_

#include <sys/types.h>
#if KERNEL
    #include <libkern/kxld_types.h>
#else
    #include "kxld_types.h"
#endif

#include "kxld_array.h"

struct kxld_array;
struct kxld_relocator;
struct kxld_reloc;
struct kxld_seg;
struct kxld_symtab;
struct relocation_info;
struct section;
struct section_64;
typedef struct kxld_sect KXLDSect;

struct kxld_sect {
	char sectname[16];          // The name of the section
	char segname[16];           // The segment to which the section belongs
	u_char *data;               // The start of the section in memory
	KXLDArray relocs;           // The section's relocation entries
	kxld_addr_t base_addr;      // The base address of the section
	kxld_addr_t link_addr;      // The relocated address of the section
	kxld_size_t size;           // The size of the section
	u_int sectnum;              // The number of the section (for relocation)
	u_int flags;                // Flags describing the section
	u_int align;                // The section's alignment as a power of 2
	u_int reserved1;            // Dependent on the section type
	u_int reserved2;            // Dependent on the section type
	boolean_t allocated;        // This section's data is allocated internally
};

/*******************************************************************************
* Constructors and destructors
*******************************************************************************/

#if KXLD_USER_OR_ILP32
/* Initializes a section object from a Mach-O section header and modifies the
 * section offset to point to the next section header.
 */
kern_return_t kxld_sect_init_from_macho_32(KXLDSect *sect, u_char *macho,
    u_long *sect_offset, u_int sectnum, const struct kxld_relocator *relocator)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_ILP32 */

#if KXLD_USER_OR_LP64
/* Initializes a section object from a Mach-O64 section header and modifies the
 * section offset to point to the next section header.
 */
kern_return_t kxld_sect_init_from_macho_64(KXLDSect *sect, u_char *macho,
    u_long *sect_offset, u_int sectnum, const struct kxld_relocator *relocator)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_LP64 */

#if KXLD_USER_OR_GOT
/* Initializes a GOT section from the number of entries that the section should
 * have.
 */
kern_return_t kxld_sect_init_got(KXLDSect *sect, u_int ngots)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_GOT */

#if KXLD_USER_OR_COMMON
/* Initializes a zerofill section of the specified size and alignment */
void kxld_sect_init_zerofill(KXLDSect *sect, const char *segname,
    const char *sectname, kxld_size_t size, u_int align)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_COMMON */

/* Clears the section object */
void kxld_sect_clear(KXLDSect *sect)
__attribute__((nonnull, visibility("hidden")));

/* Denitializes the section object and frees its array of relocs */
void kxld_sect_deinit(KXLDSect *sect)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Accessors
*******************************************************************************/

/* Gets the number of relocation entries in the section */
u_int kxld_sect_get_num_relocs(const KXLDSect *sect)
__attribute__((pure, nonnull, visibility("hidden")));

/* Returns the address parameter adjusted to the minimum alignment required by
 * the section.
 */
kxld_addr_t kxld_sect_align_address(const KXLDSect *sect, kxld_addr_t address)
__attribute__((pure, nonnull, visibility("hidden")));

/* Returns the space required by the exported Mach-O header */
u_long kxld_sect_get_macho_header_size(boolean_t is_32_bit)
__attribute__((const, visibility("hidden")));

/* Returns the space required by the exported Mach-O data */
u_long kxld_sect_get_macho_data_size(const KXLDSect *sect)
__attribute__((pure, nonnull, visibility("hidden")));

#if KXLD_USER_OR_LP64
/* Returns the number of GOT entries required by relocation entries in the
 * given section.
 */
u_int kxld_sect_get_ngots(const KXLDSect *sect,
    const struct kxld_relocator *relocator, const struct kxld_symtab *symtab)
__attribute__((pure, nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_LP64 */

kern_return_t kxld_sect_export_macho_to_file_buffer(const KXLDSect *sect, u_char *buf,
    u_long *header_offset, u_long header_size, u_long *data_offset,
    u_long data_size, boolean_t is_32_bit)
__attribute__((nonnull, visibility("hidden")));

kern_return_t kxld_sect_export_macho_to_vm(const KXLDSect *sect, u_char *buf,
    u_long *header_offset,
    u_long header_size,
    kxld_addr_t link_addr,
    u_long data_size,
    boolean_t is_32_bit)
__attribute__((nonnull, visibility("hidden")));

/*******************************************************************************
* Mutators
*******************************************************************************/

/* Relocates the section to the given link address */
void kxld_sect_relocate(KXLDSect *sect, kxld_addr_t link_addr)
__attribute__((nonnull, visibility("hidden")));

#if KXLD_USER_OR_COMMON
/* Adds a number of bytes to the section's size.  Returns the size of the
 * section before it was grown.
 */
kxld_size_t kxld_sect_grow(KXLDSect *sect, kxld_size_t nbytes, u_int align)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_COMMON */

#if KXLD_USER_OR_GOT
/* Popluates the entries of a GOT section */
kern_return_t kxld_sect_populate_got(KXLDSect *sect, struct kxld_symtab *symtab,
    boolean_t swap)
__attribute__((nonnull, visibility("hidden")));
#endif /* KXLD_USER_OR_GOT */

/* Processes all of a section's relocation entries */
kern_return_t kxld_sect_process_relocs(KXLDSect *sect,
    struct kxld_relocator *relocator)
__attribute__((nonnull, visibility("hidden")));

#endif /* _KXLD_SECT_H_ */
