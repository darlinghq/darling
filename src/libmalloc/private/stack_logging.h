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
#ifndef _STACK_LOGGING_H_
#define _STACK_LOGGING_H_

#import <stdbool.h>
#import <malloc/malloc.h>
#import <mach/vm_statistics.h>
#import <sys/cdefs.h>
#import <os/availability.h>


/*********	MallocStackLogging permanant SPIs  ************/

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

typedef void(malloc_logger_t)(uint32_t type,
							  uintptr_t arg1,
							  uintptr_t arg2,
							  uintptr_t arg3,
							  uintptr_t result,
							  uint32_t num_hot_frames_to_skip);
extern malloc_logger_t *malloc_logger;

/*
 * Load the MallocStackLogging library and register it with libmalloc
 */
boolean_t malloc_register_stack_logger(void);


/*********	MallocStackLogging deprecated SPIs  ************
 *
 * Everything here should be considered deprecated and slated for being deleted.
 * Move over to the equivilant in MallocStackLogging.h
 */

#define STACK_LOGGING_MAX_STACK_SIZE 512

#define STACK_LOGGING_VM_USER_TAG(flags) (((flags) & VM_FLAGS_ALIAS_MASK) >> 24)

/* Macro used to disguise addresses so that leak finding can work */
#define STACK_LOGGING_DISGUISE(address)	((address) ^ 0x00005555) /* nicely idempotent */

typedef enum {
	stack_logging_mode_none = 0,
	stack_logging_mode_all,
	stack_logging_mode_malloc,
	stack_logging_mode_vm,
	stack_logging_mode_lite,
	stack_logging_mode_vmlite
} stack_logging_mode_type;

extern boolean_t turn_on_stack_logging(stack_logging_mode_type mode);
extern void turn_off_stack_logging(void);

/* constants for enabling/disabling malloc stack logging via the memorystatus_vm_pressure_send sysctl */
#define	MEMORYSTATUS_ENABLE_MSL_MALLOC		0x10000000
#define MEMORYSTATUS_ENABLE_MSL_VM			0x20000000
#define MEMORYSTATUS_ENABLE_MSL_LITE		0x40000000
#define MEMORYSTATUS_DISABLE_MSL			0x80000000
#define MEMORYSTATUS_ENABLE_MSL_LITE_FULL	(MEMORYSTATUS_ENABLE_MSL_LITE | MEMORYSTATUS_ENABLE_MSL_VM | MEMORYSTATUS_ENABLE_MSL_MALLOC)
#define MEMORYSTATUS_ENABLE_MSL_LITE_VM		(MEMORYSTATUS_ENABLE_MSL_LITE | MEMORYSTATUS_ENABLE_MSL_VM)



/* 64-bit-aware stack log access.  As new SPI, these routines are prefixed with double-underscore to avoid conflict with Libsystem clients. */

typedef struct mach_stack_logging_record {
	uint32_t		type_flags;
	uint64_t		stack_identifier;
	uint64_t		argument;
	mach_vm_address_t	address;
} mach_stack_logging_record_t;


extern kern_return_t __mach_stack_logging_start_reading(task_t task, vm_address_t shared_memory_address, boolean_t *uses_lite_mode)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_start_reading",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));
extern kern_return_t __mach_stack_logging_stop_reading(task_t task)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_stop_reading",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));

/* Clients *should* call these start/stop functions to properly initialize stack logging data
 * structures and fully clean them up when they're done looking at a process.  If the client does *not*
 * call these then currently it should still work but some data structures will still remain after
 * reading the stack logs (e.g., an extra shared memory segment, an open stack log file, etc).
 * NULL can be passed for uses_lite_mode if the client doesn’t need them.
 *
 * It is recommended that the client suspend the task before actually reading the stacks, and resume the task when done,
 * if the task uses lite mode.
 */

extern kern_return_t __mach_stack_logging_set_file_path(task_t task, char* file_path)
	API_DEPRECATED("No longer supported",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));

extern kern_return_t __mach_stack_logging_get_frames(task_t task, mach_vm_address_t address, mach_vm_address_t *stack_frames_buffer, uint32_t max_stack_frames, uint32_t *count)
	API_DEPRECATED("No longer supported",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));
    /* Gets the last allocation record (malloc, realloc, or free) about address */

extern kern_return_t __mach_stack_logging_enumerate_records(task_t task, mach_vm_address_t address, void enumerator(mach_stack_logging_record_t, void *), void *context)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_disk_stack_logs_enumerate_from_task",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));
    /* Applies enumerator to all records involving address sending context as enumerator's second parameter; if !address, applies enumerator to all records */

extern kern_return_t __mach_stack_logging_frames_for_uniqued_stack(task_t task, uint64_t stack_identifier, mach_vm_address_t *stack_frames_buffer, uint32_t max_stack_frames, uint32_t *count)
	API_DEPRECATED("use __mach_stack_logging_get_frames_for_stackid instead", macos(10.9, 10.13), ios(7.0, 11.0), watchos(1.0, 4.0), tvos(9.0, 11.0));
    /* Given a uniqued_stack fills stack_frames_buffer. */

extern kern_return_t __mach_stack_logging_get_frames_for_stackid(task_t task, uint64_t stack_identifier, mach_vm_address_t *stack_frames_buffer, uint32_t max_stack_frames, uint32_t *count,
                                                                     bool *last_frame_is_threadid)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_get_frames_for_stackid",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));
    /* Given a uniqued_stack fills stack_frames_buffer. */

extern uint64_t __mach_stack_logging_stackid_for_vm_region(task_t task, mach_vm_address_t address)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_stackid_for_vm_region",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));
	/* given the address of a vm region, lookup it's stackid */


struct backtrace_uniquing_table;

extern kern_return_t
__mach_stack_logging_uniquing_table_read_stack(struct backtrace_uniquing_table *uniquing_table,
											   uint64_t stackid,
											   mach_vm_address_t *out_frames_buffer,
											   uint32_t *out_frames_count,
											   uint32_t max_frames)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_uniquing_table_read_stack",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));

extern
struct backtrace_uniquing_table *
__mach_stack_logging_copy_uniquing_table(task_t task)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_uniquing_table_copy_from_task",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));
/* returns a retained pointer to copy of the task's uniquing table */

extern
void
__mach_stack_logging_uniquing_table_release(struct backtrace_uniquing_table *)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_uniquing_table_release",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));

extern
void
__mach_stack_logging_uniquing_table_retain(struct backtrace_uniquing_table *)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_uniquing_table_retain",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));

extern
size_t
__mach_stack_logging_uniquing_table_sizeof(struct backtrace_uniquing_table *)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_uniquing_table_sizeof",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));
/* returns the serialized size of a uniquing talbe in bytes */

extern
void *
__mach_stack_logging_uniquing_table_serialize(struct backtrace_uniquing_table *table, mach_vm_size_t *size)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_uniquing_table_serialize",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));
/* Writes out a serialized representation of the table.  Free it with mach_vm_deallocate. */

extern
struct backtrace_uniquing_table *
__mach_stack_logging_uniquing_table_copy_from_serialized(void *buffer, size_t size)
	API_DEPRECATED("use MallocStackLogging/MallocStackLogging.h:msl_uniquing_table_copy_from_serialized",  macos(10.9, 10.15), ios(7.0, 13.0), watchos(1.0, 6.0), tvos(9.0, 13.0));
/* creates a malloc uniquing table from a serialized representation */

/*
 * thread_stack_pcs is now declared in its own header file, <thread_stack_pcs.h>
 */

#endif // _STACK_LOGGING_H_
