/*
 * Copyright (c) 1999, 2000, 2002-2008 Apple Inc. All rights reserved.
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

/*	Bertrand from vmutils -> CF -> System */

#import "stack_logging.h"
#import "malloc_printf.h"

#import <libc.h>
#import <pthread.h>
#import <mach/mach.h>
#include <mach/vm_statistics.h>
#import <malloc/malloc.h>
#import <stdlib.h>
#import <CrashReporterClient.h>

extern void spin_lock(int *);
extern void spin_unlock(int *);
extern void thread_stack_pcs(vm_address_t *, unsigned, unsigned *);

static inline void *allocate_pages(unsigned) __attribute__((always_inline));
static inline void *allocate_pages(unsigned bytes) {
    void *address;
    if (vm_allocate(mach_task_self(), (vm_address_t *)&address, bytes, 
                    VM_MAKE_TAG(VM_MEMORY_ANALYSIS_TOOL)| TRUE)) {
	malloc_printf("*** out of memory while stack logging\n");
	CRSetCrashLogMessage("*** out of memory while stack logging\n");
	abort();
    } 
    return (void *)address;
}

static inline void deallocate_pages(void *, unsigned) __attribute__((always_inline));
static inline void deallocate_pages(void *ptr, unsigned bytes) {
    vm_deallocate(mach_task_self(), (vm_address_t)ptr, bytes);
}

static inline void copy_pages(const void *, void *, unsigned) __attribute__((always_inline));
static inline void copy_pages(const void *source, void *dest, unsigned bytes) {
    if (vm_copy(mach_task_self(), (vm_address_t)source, bytes, (vm_address_t)dest)) memmove(dest, source, bytes);
}

/***************	Uniquing stack		***********/

#define MAX_COLLIDE	8

#define MAX_NUM_PC	512

static int enter_pair_in_table(unsigned *table, unsigned numPages, unsigned *uniquedParent, unsigned thisPC) {
    // uniquedParent is in-out; return 1 is collisions max not exceeded
    unsigned	base = numPages * vm_page_size / (sizeof(int)*2*2);
    unsigned	hash = base + (((*uniquedParent) << 4) ^ (thisPC >> 2)) % (base - 1); // modulo odd number for hashing
    unsigned	collisions = MAX_COLLIDE;
    while (collisions--) {
        unsigned	*head = table + hash*2;
        if (! head[0] && !head[1]) {
            /* end of chain; store this entry! */
            /* Note that we need to test for both head[0] and head[1] as (0, -1) is a valid entry */
            head[0] = thisPC;
            head[1] = *uniquedParent;
            *uniquedParent = hash;
            return 1;
        }
        if ((head[0] == thisPC) && (head[1] == *uniquedParent)) {
            /* we found the proper entry, the value for the pair is the entry offset */
            *uniquedParent = hash;
            return 1;
        }
        hash++;
        if (hash == base*2) hash = base;
    }
    return 0;
}

unsigned stack_logging_get_unique_stack(unsigned **table, unsigned *table_num_pages, unsigned *stack_entries, unsigned count, unsigned num_hot_to_skip) {
    unsigned	uniquedParent = (unsigned)-1;
    // we skip the warmest entries that are an artefact of the code
    while (num_hot_to_skip--) {
        if (count > 0) { stack_entries++; count--; }
    }
    while (count--) {
        unsigned	thisPC = stack_entries[count];
        while (!enter_pair_in_table(*table, *table_num_pages, &uniquedParent, thisPC)) {
            unsigned	*newTable;
            unsigned	oldBytes = (*table_num_pages) * vm_page_size;
            newTable = allocate_pages(oldBytes*2);
            copy_pages(*table, newTable, oldBytes);
            deallocate_pages(*table, oldBytes);
            *table_num_pages *= 2;
            *table = newTable;
        }
    }
    return uniquedParent;
}

/***************	Logging stack and arguments		***********/

stack_logging_record_list_t *stack_logging_the_record_list = NULL;

int stack_logging_enable_logging = 0;
int stack_logging_dontcompact = 0;
int stack_logging_finished_init = 0;
int stack_logging_postponed = 0;

static int stack_logging_spin_lock = 0;

static stack_logging_record_list_t *GrowLogRecords(stack_logging_record_list_t *records, unsigned desiredNumRecords) {
    stack_logging_record_list_t	*new_records;
    unsigned	old_size = records->overall_num_bytes;
    if (desiredNumRecords*sizeof(stack_logging_record_t)+sizeof(stack_logging_record_list_t) < records->overall_num_bytes) return records;
    records->overall_num_bytes += records->overall_num_bytes + vm_page_size; // in order to always get an even number of pages
    new_records = allocate_pages(records->overall_num_bytes);
    copy_pages(records, new_records, old_size);
    deallocate_pages(records, old_size);
    return new_records;
}

static void prepare_to_log_stack(void) {
    if (!stack_logging_the_record_list) {
        unsigned	totalSize = 4 * vm_page_size;
        stack_logging_the_record_list = allocate_pages(totalSize);
        memset(stack_logging_the_record_list, 0, sizeof(stack_logging_record_list_t));
        stack_logging_the_record_list->overall_num_bytes = totalSize;
        stack_logging_the_record_list->uniquing_table_num_pages = 128;
        stack_logging_the_record_list->uniquing_table = allocate_pages(stack_logging_the_record_list->uniquing_table_num_pages * vm_page_size);
    }
}

void stack_logging_log_stack(unsigned type, unsigned arg1, unsigned arg2, unsigned arg3, unsigned result, unsigned num_hot_to_skip) {
    stack_logging_record_t	*rec;
    if (!stack_logging_enable_logging) return;
    // printf("stack_logging_log_stack 0x%x 0x%x 0x%x 0x%x -> 0x%x\n", type, arg1, arg2, arg3, result);
    if (type & stack_logging_flag_zone) {
        // just process it now and be done with it!
        arg1 = arg2; arg2 = arg3; arg3 = 0; type &= ~stack_logging_flag_zone;
    }
    if (type & stack_logging_flag_calloc) {
        // just process it now and be done with it!
        arg1 *= arg2; arg2 = arg3; arg3 = 0; type &= ~stack_logging_flag_calloc;
    }
    if (type & stack_logging_flag_object) {
        unsigned	*class = (unsigned *)(uintptr_t)arg1;
        arg1 = arg2 + class[5]; // corresponds to the instance_size field
        arg2 = 0; arg3 = 0; type = stack_logging_type_alloc;
    }
    if (type & stack_logging_flag_cleared) {
        type &= ~stack_logging_flag_cleared;
    }
    if (type & stack_logging_flag_handle) {
        if (stack_logging_type_alloc) {
            if (!result) return;
            stack_logging_log_stack(stack_logging_type_alloc, 0, 0, 0, result, num_hot_to_skip+1);
            stack_logging_log_stack(stack_logging_type_alloc, arg1, 0, 0, *((int *)(uintptr_t)result), num_hot_to_skip+1);
            return;
        }
        if (stack_logging_type_dealloc) {
            if (!arg1) return;
            stack_logging_log_stack(stack_logging_type_dealloc, *((int *)(uintptr_t)arg1), 0, 0, 0, num_hot_to_skip+1);
            stack_logging_log_stack(stack_logging_type_dealloc, arg1, 0, 0, 0, num_hot_to_skip+1);
            return;
        }
        fprintf(stderr, "*** Unknown logging type: 0x%x\n", type);
    }
    if (type == stack_logging_flag_set_handle_size) {
        if (!arg1) return;
        // Thanks to a horrible hack, arg3 contains the prvious handle value
        if (arg3 == *((int *)(uintptr_t)arg1)) return;
        stack_logging_log_stack(stack_logging_type_dealloc, arg3, 0, 0, 0, num_hot_to_skip+1);
        stack_logging_log_stack(stack_logging_type_alloc, arg2, 0, 0, *((int *)(uintptr_t)arg1), num_hot_to_skip+1);
        return;
    }            
    if (type == (stack_logging_type_dealloc|stack_logging_type_alloc)) {
        if (arg1 == result) return; // realloc had no effect, skipping
        if (!arg1) {
            // realloc(NULL, size) same as malloc(size)
            type = stack_logging_type_alloc; arg1 = arg2; arg2 = arg3; arg3 = 0;
	} else {
            // realloc(arg1, arg2) -> result is same as free(arg1); malloc(arg2) -> result
            stack_logging_log_stack(stack_logging_type_dealloc, arg1, 0, 0, 0, num_hot_to_skip+1);
            stack_logging_log_stack(stack_logging_type_alloc, arg2, 0, 0, result, num_hot_to_skip+1);
            return;
        }
    }
    if (type == stack_logging_type_dealloc) {
        // simple free
        if (!arg1) return; // free(nil)
    }
    prepare_to_log_stack();
    spin_lock(&stack_logging_spin_lock);
    stack_logging_the_record_list = GrowLogRecords(stack_logging_the_record_list, stack_logging_the_record_list->num_records + 1);
    rec = stack_logging_the_record_list->records + stack_logging_the_record_list->num_records;
    // We take care of the common case of alloc-dealloc
    if (!stack_logging_dontcompact && stack_logging_the_record_list->num_records && (type == stack_logging_type_dealloc) && arg1 && ((rec-1)->type == stack_logging_type_alloc) && (arg1 == STACK_LOGGING_DISGUISE((rec-1)->address))) {
        stack_logging_the_record_list->num_records--;
        // printf("Erased previous record in alloc-dealloc sequence\n");
    } else {
        unsigned	stack_entries[MAX_NUM_PC];
        unsigned	count = 0;
        rec->type = type;
        if (type == stack_logging_type_dealloc) {
            rec->argument = 0;
            rec->address = STACK_LOGGING_DISGUISE(arg1); // we disguise the address
        } else if (type == stack_logging_type_alloc) {
            rec->argument = arg1;
            rec->address = STACK_LOGGING_DISGUISE(result); // we disguise the address
        } else {
            rec->argument = arg2;
            rec->address = STACK_LOGGING_DISGUISE(arg1); // we disguise the address
        }
	// printf("Before getting samples  0x%x 0x%x 0x%x 0x%x -> 0x%x\n", type, arg1, arg2, arg3, result);
        thread_stack_pcs((vm_address_t *)stack_entries, MAX_NUM_PC - 1, &count);
        // We put at the bottom of the stack a marker that denotes the thread (+1 for good measure...)
        stack_entries[count++] = (int)(uintptr_t)pthread_self() + 1;
        /* now let's unique the sample */    
        // printf("Uniquing 0x%x 0x%x 0x%x 0x%x -> 0x%x\n", type, arg1, arg2, arg3, result);
        rec->uniqued_stack = stack_logging_get_unique_stack(&stack_logging_the_record_list->uniquing_table, &stack_logging_the_record_list->uniquing_table_num_pages, stack_entries, count, num_hot_to_skip+2); // we additionally skip the warmest 2 entries that are an artefact of the code
        stack_logging_the_record_list->num_records++;
    }
    spin_unlock(&stack_logging_spin_lock);
}

static kern_return_t default_reader(task_t task, vm_address_t address, vm_size_t size, void **ptr) {
    *ptr = (void *)address;
    return 0;
}

static kern_return_t get_remote_records(task_t task, memory_reader_t reader, stack_logging_record_list_t **records) {
    // sets records
    vm_address_t	*remote_records_address_ref;
    kern_return_t	err;
    *records = NULL;
    err = reader(task, (vm_address_t)&stack_logging_the_record_list, sizeof(vm_address_t), (void **)&remote_records_address_ref);
    if (err) return err;
    if (!*remote_records_address_ref) {
        // printf("stack_logging: no stack record\n");
        return 0;
    }
    // printf("stack_logging: stack records at %p\n", (void *)(*remote_records_address_ref));
    // printf("stack_logging: reading %d bytes\n", sizeof(stack_logging_record_list_t));
    err = reader(task, *remote_records_address_ref, sizeof(stack_logging_record_list_t), (void **)records); // get the list head
    if (err) return err;
    // printf("stack_logging: overall num bytes = %d\n", records->overall_num_bytes);
    return reader(task, *remote_records_address_ref, (*records)->overall_num_bytes, (void **)records);
}

kern_return_t stack_logging_get_frames(task_t task, memory_reader_t reader, vm_address_t address, vm_address_t *stack_frames_buffer, unsigned max_stack_frames, unsigned *num_frames) {
    stack_logging_record_list_t	*records;
    kern_return_t	err;
    unsigned		index;
    unsigned		disguised = STACK_LOGGING_DISGUISE(address);
    if (!reader) reader = default_reader;
    *num_frames = 0;
    err = get_remote_records(task, reader, &records);
    if (err || !records) return err;
    // printf("stack_logging: %d records\n", records->num_records);
    index = 0;
    while (index < records->num_records) {
        stack_logging_record_t	*record = records->records + index;
        if (record->address == disguised) {
	    return stack_logging_frames_for_uniqued_stack(task, reader, record->uniqued_stack, stack_frames_buffer, max_stack_frames, num_frames);
        }
	index++;
    }
    fprintf(stderr, "*** stack_logging: no record found for %p\n", address);
    return 0;
}

kern_return_t stack_logging_enumerate_records(task_t task, memory_reader_t reader, vm_address_t address, void enumerator(stack_logging_record_t, void *), void *context) {
    stack_logging_record_list_t	*records;
    kern_return_t	err;
    unsigned		index;
    unsigned		disguised = STACK_LOGGING_DISGUISE(address);
    if (!reader) reader = default_reader;
    err = get_remote_records(task, reader, &records);
    if (err || !records) return err;
    // printf("stack_logging: %d records\n", records->num_records);
    index = 0;
    while (index < records->num_records) {
        stack_logging_record_t	*record = records->records + index;
        if (!address || (record->address == disguised)) enumerator(*record, context);
	index++;
    }
    return 0;
}

kern_return_t stack_logging_frames_for_uniqued_stack(task_t task, memory_reader_t reader, unsigned uniqued_stack, vm_address_t *stack_frames_buffer, unsigned max_stack_frames, unsigned *num_frames) {
    stack_logging_record_list_t	*records;
    unsigned		*uniquing_table;
    kern_return_t	err;
    if (!reader) reader = default_reader;
    *num_frames = 0;
    err = get_remote_records(task, reader, &records);
    if (err || !records) return err;
    err = reader(task, (vm_address_t)records->uniquing_table, records->uniquing_table_num_pages * vm_page_size, (void **)&uniquing_table);
    if (err) return err;
    while (max_stack_frames && (uniqued_stack != -1)) {
	unsigned	thisPC;
	if ((uniqued_stack * 2 + 1) * sizeof(unsigned) >= records->uniquing_table_num_pages * vm_page_size) {
	    fprintf(stderr, "*** stack_logging: Invalid uniqued stack 0x%x", uniqued_stack);
	    break;
	}
	thisPC = uniquing_table[uniqued_stack * 2];
	uniqued_stack = uniquing_table[uniqued_stack * 2 + 1];
	if (!thisPC && !uniqued_stack) {
	    // Invalid entry
	    fprintf(stderr, "*** stack_logging: Invalid entry 0x%x", thisPC);
	    break;
	}
	stack_frames_buffer[0] = thisPC;
	stack_frames_buffer++;
	(*num_frames)++;
	max_stack_frames--;
    }
    return 0;
}
