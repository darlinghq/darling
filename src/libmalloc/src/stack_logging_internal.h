/*
 * Copyright (c) 2016, Apple Inc. All rights reserved.
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

#ifndef stack_logging_disk_h
#define stack_logging_disk_h

extern int stack_logging_enable_logging; /* when clear, no logging takes place */
extern int stack_logging_dontcompact; /* default is to compact; when set does not compact alloc/free logs; useful for tracing history */
extern int stack_logging_finished_init; /* set after we've returned from the Libsystem initialiser */
extern int stack_logging_postponed; /* set if we needed to postpone logging till after initialisation */
extern int stack_logging_mode;

extern const uint64_t __invalid_stack_id;

// returns the stack id
uint64_t __enter_stack_into_table_while_locked(vm_address_t self_thread, uint32_t num_hot_to_skip, boolean_t add_thread_id, size_t ptr_size);

void __malloc_lock_stack_logging();
void __malloc_unlock_stack_logging();

// support for gdb and others checking for stack_logging locks
extern boolean_t __stack_logging_locked();

extern boolean_t __prepare_to_log_stacks(boolean_t lite_mode);
extern void __prepare_to_log_stacks_stage2();

// support for multi-threaded forks
extern void __stack_logging_fork_prepare();
extern void __stack_logging_fork_parent();
extern void __stack_logging_fork_child();
extern void __stack_logging_early_finished();

void __decrement_table_slot_refcount(uint64_t stackID, size_t size);

void __delete_uniquing_table_memory_while_locked();
boolean_t __uniquing_table_memory_was_deleted(void);

boolean_t is_stack_logging_lite_enabled(void);


#endif /* stack_logging_disk_h */
