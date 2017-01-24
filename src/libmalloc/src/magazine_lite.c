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

#include "internal.h"

typedef uint64_t malloc_stack_id;

uint64_t max_lite_mallocs = 0;

static malloc_stack_id
get_stack_id_from_ptr(void *ptr, size_t ptr_size)
{
	void *idptr = ptr + ptr_size - sizeof(malloc_stack_id);
	
	return * (malloc_stack_id *) idptr;
}

static void
set_stack_id_in_ptr(void *ptr, size_t requested_size, size_t ptr_size, malloc_stack_id	stack_id)
{
	void *idptr = ptr + ptr_size - sizeof(malloc_stack_id);
	* (malloc_stack_id *) idptr = stack_id;
	void *padding = ptr + requested_size;
	bzero(padding, ptr_size - requested_size - sizeof(malloc_stack_id));
}

static void
add_stack_to_ptr(szone_t *szone, size_t requested_size, void *ptr)
{
	// enter the stack into the unique table
	vm_address_t self_thread = (vm_address_t)_os_tsd_get_direct(__TSD_THREAD_SELF);
	
	size_t ptr_size = szone_size(szone, ptr);
	
	__malloc_lock_stack_logging();
	
	// if stack logging was turned off behind our backs
	if (!is_stack_logging_lite_enabled()) {
		__malloc_unlock_stack_logging();
		return;
	}

	malloc_stack_id stack_id = __enter_stack_into_table_while_locked(self_thread, 0, false, ptr_size);
	
	__malloc_unlock_stack_logging();
	
	if (stack_id == __invalid_stack_id) {
		malloc_printf("bad stack id. turning off stack logging\n");
		turn_off_stack_logging();
	} else {
		set_stack_id_in_ptr(ptr, requested_size, ptr_size, stack_id);
	}
}

static boolean_t stack_logging_lite_enabled = false;

boolean_t is_stack_logging_lite_enabled(void) {
	return stack_logging_lite_enabled;
}

void
enable_stack_logging_lite()
{
	stack_logging_lite_enabled = true;
}

void
disable_stack_logging_lite()
{
	stack_logging_lite_enabled = false;
}

static void *
stack_logging_lite_malloc(malloc_zone_t *zone, size_t size)
{
	szone_t *szone = (szone_t *) zone;
	void* p = NULL;
	static uint64_t num_mallocs = 0;
	
	if (stack_logging_lite_enabled) {
		__prepare_to_log_stacks(true);	// do this again in case stack logging was postponed
		
		p = szone_malloc(szone, size + sizeof(malloc_stack_id));
		
		if (p) {
			add_stack_to_ptr(szone, size, p);
		}
		
		// this value doesn't need to be exact, so no need for atomic operations
		num_mallocs++;
		
		if (max_lite_mallocs > 0 && num_mallocs > max_lite_mallocs) {
			malloc_printf("lite allocations exceeded limit. disabling lite mode\n");
			disable_stack_logging_lite();
		}
	} else {
		p = szone->helper_zone->basic_zone.malloc((malloc_zone_t *) szone->helper_zone, size);
	}
	
	return p;
}

static void *
stack_logging_lite_calloc(struct _malloc_zone_t *zone, size_t num_items, size_t size)
{
	szone_t *szone = (szone_t *) zone;
	void *p = NULL;
	
	if (stack_logging_lite_enabled) {
		size_t total_bytes = (num_items * size) + sizeof(malloc_stack_id);
		
		if (num_items > 1) {
			
#if __LP64__ /* size_t is uint64_t */
			if ((num_items | size) & 0xffffffff00000000ul) {
				// num_items or size equals or exceeds sqrt(2^64) == 2^32, appeal to wider arithmetic
				__uint128_t product = (((__uint128_t)num_items) * ((__uint128_t)size)) + sizeof(malloc_stack_id);
				if ((uint64_t)(product >> 64)) { // compiles to test on upper register of register pair
					return NULL;
				}
			}
#else /* size_t is uint32_t */
			if ((num_items | size) & 0xffff0000ul) {
				// num_items or size equals or exceeds sqrt(2^32) == 2^16, appeal to wider arithmetic
				uint64_t product = ((uint64_t)num_items) * ((uint64_t)size) + sizeof(malloc_stack_id);;
				if ((uint32_t)(product >> 32)) { // compiles to test on upper register of register pair
					return NULL;
				}
			}
#endif
			
		}
		
		p = szone_malloc_should_clear(szone, total_bytes, 1);
		
		if (p) {
			add_stack_to_ptr(szone, num_items * size, p);
		}
	} else {
		p = szone->helper_zone->basic_zone.calloc((malloc_zone_t *) szone->helper_zone, num_items, size);
	}
	
	return p;
}

static void *
stack_logging_lite_valloc(malloc_zone_t *zone, size_t size)
{
	szone_t *szone = (szone_t *) zone;
	void *p = NULL;
	
	if (stack_logging_lite_enabled) {
		p = szone_valloc(szone, size + sizeof(malloc_stack_id));
		
		if (p) {
			add_stack_to_ptr(szone, size, p);
		}
	} else {
		p = szone->helper_zone->basic_zone.valloc((malloc_zone_t *) szone->helper_zone, size);
	}
	
	return p;
}

static void
stack_logging_lite_free(malloc_zone_t *zone, void *ptr)
{
	szone_t *szone = (szone_t *) zone;
	size_t size = szone_size(szone, ptr);
	
	// see if it's in our zone
	if (size) {
		malloc_stack_id stack_id = get_stack_id_from_ptr(ptr, size);
		__decrement_table_slot_refcount(stack_id, size);
		szone_free(szone, ptr);
	} else {
		szone->helper_zone->basic_zone.free((malloc_zone_t *) szone->helper_zone, ptr);
	}
}

static void
stack_logging_lite_free_definite_size(malloc_zone_t *zone, void *ptr, size_t size)
{
	// because we've messed with the size, don't try to be fancy and just call free
	stack_logging_lite_free(zone, ptr);
}

// Three paths:
// 1. do a szone_realloc with padding and add stack id
// 2. do a szone_realloc on the helper zone
// 3. do a manual free / malloc

static void *
stack_logging_lite_realloc(malloc_zone_t *zone, void *ptr, size_t new_size)
{
	szone_t *szone = (szone_t *) zone;
	void *new_ptr = NULL;
	
	size_t old_size = szone_size(szone, ptr);
	
	// if we own the ptr and lite enabled do our thing
	if (old_size && stack_logging_lite_enabled) {
		// need to get the old stackid and decrement
		malloc_stack_id stack_id = get_stack_id_from_ptr(ptr, old_size);
		new_ptr = szone_realloc(szone, ptr, new_size + sizeof(malloc_stack_id));
		
		if (new_ptr) {
			__decrement_table_slot_refcount(stack_id, old_size);
			add_stack_to_ptr(szone, new_size, new_ptr);
		}
	} else if (!old_size && !stack_logging_lite_enabled) {
		// we don't own the pointer and lite mode is disabled, so just pass the realloc on to the helper zone
		return szone->helper_zone->basic_zone.realloc((malloc_zone_t *) szone->helper_zone, ptr, new_size);
	} else {
		// otherwise perform the realloc by hand:
		// 1. malloc new ptr
		// 2. copy existing data to new ptr
		// 3. free old ptr
		
		// this will add the stack id if needed
		new_ptr = stack_logging_lite_malloc(zone, new_size);
		
		if (new_ptr) {
			size_t old_size = malloc_size(ptr);
			size_t new_size = malloc_size(new_ptr);
			
			// copy as much old data as possible
			size_t copy_size = MIN(old_size, new_size);
			memcpy(new_ptr, ptr, copy_size);
		}
		
		stack_logging_lite_free(zone, ptr);
	}
	
	return new_ptr;
}

static void * MALLOC_NOINLINE
stack_logging_lite_memalign(malloc_zone_t *zone, size_t alignment, size_t size)
{
	szone_t *szone = (szone_t *) zone;
	void *ptr = NULL;
	
	if (stack_logging_lite_enabled) {
		ptr = szone_memalign(szone, alignment, size + sizeof(malloc_stack_id));
		
		if (ptr) {
			add_stack_to_ptr(szone, size, ptr);
		}
	} else {
		ptr = szone->helper_zone->basic_zone.memalign((malloc_zone_t *) szone->helper_zone, alignment, size);
	}
	
	return ptr;
}

static size_t
stack_logging_lite_size(malloc_zone_t *zone, const void *ptr)
{
	szone_t *szone = (szone_t *) zone;
	size_t size = szone_size(szone, ptr);
	
	if (size) {
		size -= sizeof(malloc_stack_id);
	} else {
		size = szone->helper_zone->basic_zone.size((malloc_zone_t *) szone->helper_zone, ptr);
	}
	
	return size;
}

unsigned
stack_logging_lite_batch_malloc(szone_t *szone, size_t size, void **results, unsigned count)
{
	unsigned num_allocated = 0;
	
	if (stack_logging_lite_enabled) {
		num_allocated = szone_batch_malloc(szone, size + sizeof(malloc_stack_id), results, count);
		
		for (unsigned i = 0; i < num_allocated; i++) {
			add_stack_to_ptr(szone, size, results[i]);
		}
	} else {
		num_allocated = szone->helper_zone->basic_zone.batch_malloc((malloc_zone_t *) szone->helper_zone, size, results, count);
	}
	
	return num_allocated;
}

void
stack_logging_lite_batch_free(szone_t *szone, void **to_be_freed, unsigned count)
{
	for (unsigned i = 0; i < count; i++) {
		void *p = to_be_freed[i];
		
		if (p) {
			size_t size = szone_size(szone, p);
			
			// see if it's in our zone
			if (size) {
				malloc_stack_id stack_id = get_stack_id_from_ptr(p, size);
				__decrement_table_slot_refcount(stack_id, size);
				szone_free(szone, p);
			} else {
				szone->helper_zone->basic_zone.free((malloc_zone_t *) szone->helper_zone, p);
			}
		}
	}
}


malloc_zone_t *
create_stack_logging_lite_zone(size_t initial_size, malloc_zone_t *helper_zone, unsigned debug_flags)
{
	szone_t* zone = create_scalable_szone(initial_size, debug_flags);
	
	// unprotect function pointers
	mprotect(zone, sizeof(zone->basic_zone), PROT_READ | PROT_WRITE);
	
	// set the function pointers
	zone->basic_zone.malloc = stack_logging_lite_malloc;
	zone->basic_zone.calloc = stack_logging_lite_calloc;
	zone->basic_zone.valloc = stack_logging_lite_valloc;
	zone->basic_zone.realloc = stack_logging_lite_realloc;
	zone->basic_zone.batch_malloc = (void *) stack_logging_lite_batch_malloc;
	zone->basic_zone.batch_free = (void *) stack_logging_lite_batch_free;
	zone->basic_zone.memalign = stack_logging_lite_memalign;
	zone->basic_zone.free = stack_logging_lite_free;
	zone->basic_zone.free_definite_size = stack_logging_lite_free_definite_size;
	zone->basic_zone.size = stack_logging_lite_size;
	
	// protect function pointers
	mprotect(zone, sizeof(zone->basic_zone), PROT_READ);
	
	// set helper zone
	zone->helper_zone = (szone_t*) helper_zone;
	
	return (malloc_zone_t*) zone;
}
