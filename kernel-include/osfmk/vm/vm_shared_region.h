/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
 *
 *	File: vm/vm_shared_region.h
 *
 *      protos and struct definitions for shared region
 */

#ifndef _VM_SHARED_REGION_H_
#define _VM_SHARED_REGION_H_

#ifdef  KERNEL_PRIVATE

#include <mach/vm_prot.h>
#include <mach/mach_types.h>
#include <mach/shared_region.h>

#include <kern/kern_types.h>
#include <kern/macro_help.h>

#include <vm/vm_map.h>

extern int shared_region_version;
extern int shared_region_persistence;

#if DEBUG
extern int shared_region_debug;
#define SHARED_REGION_DEBUG(args)               \
	MACRO_BEGIN                             \
	if (shared_region_debug) {              \
	        kprintf args;                   \
	}                                       \
	MACRO_END
#else /* DEBUG */
#define SHARED_REGION_DEBUG(args)
#endif /* DEBUG */

extern int shared_region_trace_level;

extern struct vm_shared_region *init_task_shared_region;

#define SHARED_REGION_TRACE_NONE_LVL            0 /* no trace */
#define SHARED_REGION_TRACE_ERROR_LVL           1 /* trace abnormal events */
#define SHARED_REGION_TRACE_INFO_LVL            2 /* trace all events */
#define SHARED_REGION_TRACE_DEBUG_LVL           3 /* extra traces for debug */
#define SHARED_REGION_TRACE(level, args)                \
	MACRO_BEGIN                                     \
	if (shared_region_trace_level >= level) {       \
	        printf args;                            \
	}                                               \
	MACRO_END
#define SHARED_REGION_TRACE_NONE(args)
#define SHARED_REGION_TRACE_ERROR(args)                         \
	MACRO_BEGIN                                             \
	SHARED_REGION_TRACE(SHARED_REGION_TRACE_ERROR_LVL,      \
	                    args);                              \
	MACRO_END
#define SHARED_REGION_TRACE_INFO(args)                          \
	MACRO_BEGIN                                             \
	SHARED_REGION_TRACE(SHARED_REGION_TRACE_INFO_LVL,       \
	                    args);                              \
	MACRO_END
#define SHARED_REGION_TRACE_DEBUG(args)                         \
	MACRO_BEGIN                                             \
	SHARED_REGION_TRACE(SHARED_REGION_TRACE_DEBUG_LVL,      \
	                    args);                              \
	MACRO_END

typedef struct vm_shared_region *vm_shared_region_t;

#ifdef MACH_KERNEL_PRIVATE

#include <kern/queue.h>
#include <vm/vm_object.h>
#include <vm/memory_object.h>

#define PAGE_SIZE_FOR_SR_SLIDE  4096

/* Documentation for the slide info format can be found in the dyld project in
 * the file 'launch-cache/dyld_cache_format.h'. */

typedef struct vm_shared_region_slide_info_entry_v1 *vm_shared_region_slide_info_entry_v1_t;
struct vm_shared_region_slide_info_entry_v1 {
	uint32_t        version;
	uint32_t        toc_offset;     // offset from start of header to table-of-contents
	uint32_t        toc_count;      // number of entries in toc (same as number of pages in r/w mapping)
	uint32_t        entry_offset;
	uint32_t        entry_count;
	// uint16_t	toc[toc_count];
	// entrybitmap	entries[entries_count];
};

#define NBBY    8
#define NUM_SLIDING_BITMAPS_PER_PAGE    (0x1000/sizeof(int)/NBBY) /*128*/
typedef struct slide_info_entry_toc     *slide_info_entry_toc_t;
struct slide_info_entry_toc {
	uint8_t entry[NUM_SLIDING_BITMAPS_PER_PAGE];
};

typedef struct vm_shared_region_slide_info_entry_v2 *vm_shared_region_slide_info_entry_v2_t;
struct vm_shared_region_slide_info_entry_v2 {
	uint32_t        version;
	uint32_t        page_size;
	uint32_t        page_starts_offset;
	uint32_t        page_starts_count;
	uint32_t        page_extras_offset;
	uint32_t        page_extras_count;
	uint64_t        delta_mask;             // which (contiguous) set of bits contains the delta to the next rebase location
	uint64_t        value_add;
	// uint16_t	page_starts[page_starts_count];
	// uint16_t	page_extras[page_extras_count];
};

#define DYLD_CACHE_SLIDE_PAGE_ATTRS             0xC000  // high bits of uint16_t are flags
#define DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA        0x8000  // index is into extras array (not starts array)
#define DYLD_CACHE_SLIDE_PAGE_ATTR_NO_REBASE    0x4000  // page has no rebasing
#define DYLD_CACHE_SLIDE_PAGE_ATTR_END          0x8000  // last chain entry for page
#define DYLD_CACHE_SLIDE_PAGE_VALUE             0x3FFF  // bitwise negation of DYLD_CACHE_SLIDE_PAGE_ATTRS
#define DYLD_CACHE_SLIDE_PAGE_OFFSET_SHIFT      2

typedef struct vm_shared_region_slide_info_entry_v3 *vm_shared_region_slide_info_entry_v3_t;
struct vm_shared_region_slide_info_entry_v3 {
	uint32_t        version;                        // currently 3
	uint32_t        page_size;                      // currently 4096 (may also be 16384)
	uint32_t        page_starts_count;
	uint64_t        value_add;
	uint16_t        page_starts[] /* page_starts_count */;
};

#define DYLD_CACHE_SLIDE_V3_PAGE_ATTR_NO_REBASE 0xFFFF  // page has no rebasing


typedef struct vm_shared_region_slide_info_entry_v4 *vm_shared_region_slide_info_entry_v4_t;
struct vm_shared_region_slide_info_entry_v4 {
	uint32_t    version;        // currently 4
	uint32_t    page_size;      // currently 4096 (may also be 16384)
	uint32_t    page_starts_offset;
	uint32_t    page_starts_count;
	uint32_t    page_extras_offset;
	uint32_t    page_extras_count;
	uint64_t    delta_mask;    // which (contiguous) set of bits contains the delta to the next rebase location (0xC0000000)
	uint64_t    value_add;     // base address of cache
	// uint16_t    page_starts[page_starts_count];
	// uint16_t    page_extras[page_extras_count];
};

#define DYLD_CACHE_SLIDE4_PAGE_NO_REBASE           0xFFFF  // page has no rebasing
#define DYLD_CACHE_SLIDE4_PAGE_INDEX               0x7FFF  // index into starts or extras
#define DYLD_CACHE_SLIDE4_PAGE_USE_EXTRA           0x8000  // index is into extras array (not starts array)
#define DYLD_CACHE_SLIDE4_PAGE_EXTRA_END           0x8000  // last chain entry for page



typedef union vm_shared_region_slide_info_entry *vm_shared_region_slide_info_entry_t;
union vm_shared_region_slide_info_entry {
	uint32_t        version;
	struct vm_shared_region_slide_info_entry_v1     v1;
	struct vm_shared_region_slide_info_entry_v2     v2;
	struct vm_shared_region_slide_info_entry_v3     v3;
	struct vm_shared_region_slide_info_entry_v4 v4;
};

typedef struct vm_shared_region_slide_info *vm_shared_region_slide_info_t;
struct vm_shared_region_slide_info {
	mach_vm_address_t       slid_address;
	mach_vm_offset_t        start;
	mach_vm_offset_t        end;
	uint32_t                slide;
#if defined(HAS_APPLE_PAC)
	boolean_t               si_ptrauth;
#endif /* HAS_APPLE_PAC */
	vm_object_t             slide_object;
	mach_vm_size_t          slide_info_size;
	vm_shared_region_slide_info_entry_t     slide_info_entry;
};

/* address space shared region descriptor */
struct vm_shared_region {
	uint32_t                sr_ref_count;
	queue_chain_t           sr_q;
	void                    *sr_root_dir;
	cpu_type_t              sr_cpu_type;
	cpu_subtype_t           sr_cpu_subtype;
	boolean_t               sr_64bit;
	boolean_t               sr_mapping_in_progress;
	boolean_t               sr_slide_in_progress;
	boolean_t               sr_persists;
	boolean_t               sr_slid;
	ipc_port_t              sr_mem_entry;
	mach_vm_offset_t        sr_first_mapping;
	mach_vm_offset_t        sr_base_address;
	mach_vm_size_t          sr_size;
	mach_vm_offset_t        sr_pmap_nesting_start;
	mach_vm_size_t          sr_pmap_nesting_size;
	thread_call_t           sr_timer_call;
	struct vm_shared_region_slide_info sr_slide_info;
	uuid_t                  sr_uuid;
	boolean_t               sr_uuid_copied;
	uint32_t                sr_images_count;
	struct dyld_uuid_info_64 *sr_images;
};

extern kern_return_t vm_shared_region_slide_page(vm_shared_region_slide_info_t si,
    vm_offset_t     vaddr,
    mach_vm_offset_t uservaddr,
    uint32_t pageIndex);
extern vm_shared_region_slide_info_t vm_shared_region_get_slide_info(vm_shared_region_t sr);
#else  /* !MACH_KERNEL_PRIVATE */

struct vm_shared_region;
struct vm_shared_region_slide_info;
struct vm_shared_region_slide_info_entry;
struct slide_info_entry_toc;

#endif /* MACH_KERNEL_PRIVATE */

extern void vm_shared_region_init(void);
extern kern_return_t vm_shared_region_enter(
	struct _vm_map          *map,
	struct task             *task,
	boolean_t               is_64bit,
	void                    *fsroot,
	cpu_type_t              cpu,
	cpu_subtype_t           cpu_subtype);
extern kern_return_t vm_shared_region_remove(
	struct _vm_map          *map,
	struct task             *task);
extern vm_shared_region_t vm_shared_region_get(
	struct task             *task);
extern vm_shared_region_t vm_shared_region_trim_and_get(
	struct task             *task);
extern void vm_shared_region_deallocate(
	struct vm_shared_region *shared_region);
extern mach_vm_offset_t vm_shared_region_base_address(
	struct vm_shared_region *shared_region);
extern mach_vm_size_t vm_shared_region_size(
	struct vm_shared_region *shared_region);
extern ipc_port_t vm_shared_region_mem_entry(
	struct vm_shared_region *shared_region);
extern vm_map_t vm_shared_region_vm_map(
	struct vm_shared_region *shared_region);
extern uint32_t vm_shared_region_get_slide(
	vm_shared_region_t      shared_region);
extern void vm_shared_region_set(
	struct task             *task,
	struct vm_shared_region *new_shared_region);
extern vm_shared_region_t vm_shared_region_lookup(
	void                    *root_dir,
	cpu_type_t              cpu,
	cpu_subtype_t           cpu_subtype,
	boolean_t               is_64bit);
extern kern_return_t vm_shared_region_start_address(
	struct vm_shared_region *shared_region,
	mach_vm_offset_t        *start_address);
extern void vm_shared_region_undo_mappings(
	vm_map_t sr_map,
	mach_vm_offset_t sr_base_address,
	struct shared_file_mapping_np *mappings,
	unsigned int mappings_count);
extern kern_return_t vm_shared_region_map_file(
	struct vm_shared_region *shared_region,
	unsigned int            mappings_count,
	struct shared_file_mapping_np *mappings,
	memory_object_control_t file_control,
	memory_object_size_t    file_size,
	void                    *root_dir,
	uint32_t                slide,
	user_addr_t             slide_start,
	user_addr_t             slide_size);
extern kern_return_t vm_shared_region_sliding_valid(uint32_t slide);
extern kern_return_t vm_shared_region_slide_sanity_check(vm_shared_region_t sr);
extern void* vm_shared_region_get_slide_info_entry(vm_shared_region_t sr);
extern void vm_commpage_init(void);
extern void vm_commpage_text_init(void);
extern kern_return_t vm_commpage_enter(
	struct _vm_map          *map,
	struct task             *task,
	boolean_t               is64bit);
extern kern_return_t vm_commpage_remove(
	struct _vm_map          *map,
	struct task             *task);
int vm_shared_region_slide(uint32_t,
    mach_vm_offset_t,
    mach_vm_size_t,
    mach_vm_offset_t,
    mach_vm_size_t,
    mach_vm_offset_t,
    memory_object_control_t);

#endif /* KERNEL_PRIVATE */

#endif  /* _VM_SHARED_REGION_H_ */
