/*
 * Copyright (c) 1999-2007 Apple Inc. All rights reserved.
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

#import <malloc/malloc.h>
#import <mach/vm_statistics.h>

#define STACK_LOGGING_MAX_STACK_SIZE 512

#define stack_logging_type_free		0
#define stack_logging_type_generic	1	/* anything that is not allocation/deallocation */
#define stack_logging_type_alloc	2	/* malloc, realloc, etc... */
#define stack_logging_type_dealloc	4	/* free, realloc, etc... */
#define stack_logging_type_vm_allocate  16      /* vm_allocate or mmap */
#define stack_logging_type_vm_deallocate  32	/* vm_deallocate or munmap */
#define stack_logging_type_mapped_file_or_shared_mem	128

// The valid flags include those from VM_FLAGS_ALIAS_MASK, which give the user_tag of allocated VM regions.
#define stack_logging_valid_type_flags ( \
	stack_logging_type_generic | \
	stack_logging_type_alloc | \
	stack_logging_type_dealloc | \
	stack_logging_type_vm_allocate | \
	stack_logging_type_vm_deallocate | \
	stack_logging_type_mapped_file_or_shared_mem | \
	VM_FLAGS_ALIAS_MASK);

// Following flags are absorbed by stack_logging_log_stack()
#define	stack_logging_flag_zone		8	/* NSZoneMalloc, etc... */
#define stack_logging_flag_cleared	64	/* for NewEmptyHandle */

#define STACK_LOGGING_VM_USER_TAG(flags) (((flags) & VM_FLAGS_ALIAS_MASK) >> 24)


/* Macro used to disguise addresses so that leak finding can work */
#define STACK_LOGGING_DISGUISE(address)	((address) ^ 0x00005555) /* nicely idempotent */

typedef enum {
	stack_logging_mode_none = 0,
	stack_logging_mode_all,
	stack_logging_mode_malloc,
	stack_logging_mode_vm,
	stack_logging_mode_lite
} stack_logging_mode_type;

extern boolean_t turn_on_stack_logging(stack_logging_mode_type mode);
extern void turn_off_stack_logging();

extern void __disk_stack_logging_log_stack(uint32_t type_flags, uintptr_t zone_ptr, uintptr_t size, uintptr_t ptr_arg, uintptr_t return_val, uint32_t num_hot_to_skip);
	/* Fits as the malloc_logger; logs malloc/free/realloc events and can log custom events if called directly */


/* 64-bit-aware stack log access.  As new SPI, these routines are prefixed with double-underscore to avoid conflict with Libsystem clients. */

typedef struct mach_stack_logging_record {
	uint32_t		type_flags;
	uint64_t		stack_identifier;
	uint64_t		argument;
	mach_vm_address_t	address;
} mach_stack_logging_record_t;

extern kern_return_t __mach_stack_logging_start_reading(task_t task, vm_address_t shared_memory_address, boolean_t *uses_lite_mode);
extern kern_return_t __mach_stack_logging_stop_reading(task_t task);

/* Clients *should* call these start/stop functions to properly initialize stack logging data
 * structures and fully clean them up when they're done looking at a process.  If the client does *not*
 * call these then currently it should still work but some data structures will still remain after
 * reading the stack logs (e.g., an extra shared memory segment, an open stack log file, etc).
 * NULL can be passed for uses_lite_mode if the client doesn’t need them.
 *
 * It is recommended that the client suspend the task before actually reading the stacks, and resume the task when done,
 * if the task uses lite mode.
 */

extern kern_return_t __mach_stack_logging_set_file_path(task_t task, char* file_path);

extern kern_return_t __mach_stack_logging_get_frames(task_t task, mach_vm_address_t address, mach_vm_address_t *stack_frames_buffer, uint32_t max_stack_frames, uint32_t *count);
    /* Gets the last allocation record (malloc, realloc, or free) about address */

extern kern_return_t __mach_stack_logging_enumerate_records(task_t task, mach_vm_address_t address, void enumerator(mach_stack_logging_record_t, void *), void *context);
    /* Applies enumerator to all records involving address sending context as enumerator's second parameter; if !address, applies enumerator to all records */

extern kern_return_t __mach_stack_logging_frames_for_uniqued_stack(task_t task, uint64_t stack_identifier, mach_vm_address_t *stack_frames_buffer, uint32_t max_stack_frames, uint32_t *count);
    /* Given a uniqued_stack fills stack_frames_buffer */


struct backtrace_uniquing_table;

extern kern_return_t
__mach_stack_logging_uniquing_table_read_stack(struct backtrace_uniquing_table *uniquing_table,
											   uint64_t stackid,
											   mach_vm_address_t *out_frames_buffer,
											   uint32_t *out_frames_count,
											   uint32_t max_frames);

extern
struct backtrace_uniquing_table *
__mach_stack_logging_copy_uniquing_table(task_t task);
/* returns a retained pointer to copy of the task's uniquing table */

extern
void
__mach_stack_logging_uniquing_table_release(struct backtrace_uniquing_table *);

extern
void
__mach_stack_logging_uniquing_table_retain(struct backtrace_uniquing_table *);

extern
size_t
__mach_stack_logging_uniquing_table_sizeof(struct backtrace_uniquing_table *);
/* returns the serialized size of a uniquing talbe in bytes */

extern
void *
__mach_stack_logging_uniquing_table_serialize(struct backtrace_uniquing_table *table, mach_vm_size_t *size);
/* Writes out a serialized representation of the table.  Free it with mach_vm_deallocate. */

extern
struct backtrace_uniquing_table *
__mach_stack_logging_uniquing_table_copy_from_serialized(void *buffer, size_t size);
/* creates a malloc uniquing table from a serialized representation */


extern void thread_stack_pcs(vm_address_t *buffer, unsigned max, unsigned *num);
    /* Convenience to fill buffer with the PCs of the frames, starting with the hot frames;
    num: returned number of frames
    */

