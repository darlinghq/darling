/*
 * Copyright (c) 1999-2016 Apple Inc. All rights reserved.
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

#ifndef _MALLOC_PRIVATE_H_
#define _MALLOC_PRIVATE_H_

/* Here be dragons (SPIs) */

#include <mach/boolean.h>
#include <sys/cdefs.h>
#include <Availability.h>
#include <os/availability.h>
#include <malloc/malloc.h>

/*********	Callbacks	************/

API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
void malloc_enter_process_memory_limit_warn_mode(void);
	/* A callback invoked once the process receives a warning for approaching
	 * memory limit. */

__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0)
__TVOS_AVAILABLE(10.0) __WATCHOS_AVAILABLE(3.0)
void malloc_memory_event_handler(unsigned long);
	/* A function invoked when malloc needs to handle any flavor of
	 * memory pressure notification or process memory limit notification. */

API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
void * reallocarray(void * in_ptr, size_t nmemb, size_t size) __DARWIN_EXTSN(reallocarray) __result_use_check;

API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
void * reallocarrayf(void * in_ptr, size_t nmemb, size_t size) __DARWIN_EXTSN(reallocarrayf) __result_use_check;

/*
 * Checks whether an address might belong to any registered zone. False positives
 * are allowed (e.g. the memory was freed, or it's in a part of the address
 * space used by malloc that has not yet been allocated.) False negatives are
 * not allowed.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
boolean_t malloc_claimed_address(void *ptr) __result_use_check;

/*
 * Checks whether an address might belong to a given zone. False positives are
 * allowed (e.g. the memory was freed, or it's in a part of the address space
 * used by malloc that has not yet been allocated.) False negatives are not
 * allowed.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
boolean_t malloc_zone_claimed_address(malloc_zone_t *zone, void *ptr) __result_use_check;

/**
 * Returns whether the nano allocator is engaged. The return value is 0 if Nano
 * is not engaged and the allocator version otherwise.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
int malloc_engaged_nano(void) __result_use_check;

#endif /* _MALLOC_PRIVATE_H_ */
