/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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

#define SCALABLE_MALLOC_ADD_GUARD_PAGES		(1 << 0)
    // add a guard page before and after each VM region to help debug
#define SCALABLE_MALLOC_DONT_PROTECT_PRELUDE	(1 << 1)
    // do not protect prelude page
#define SCALABLE_MALLOC_DONT_PROTECT_POSTLUDE	(1 << 2)
    // do not protect postlude page
#define SCALABLE_MALLOC_DO_SCRIBBLE		(1 << 3)
    // write 0x55 onto free blocks
#define SCALABLE_MALLOC_ABORT_ON_ERROR (1 << 4)
    // call abort() on any malloc error, such as double free or out of memory.
#define SCALABLE_MALLOC_PURGEABLE (1 << 5)
    // allocate objects such that they may be used with VM purgability APIs
#define SCALABLE_MALLOC_ABORT_ON_CORRUPTION (1 << 6)
    // call abort() on malloc errors, but not on out of memory.

extern malloc_zone_t *create_scalable_zone(size_t initial_size, unsigned debug_flags);
    /* Create a new zone that scales for small objects or large objects */

extern malloc_zone_t *create_purgeable_zone(size_t initial_size, malloc_zone_t *malloc_default_zone, unsigned debug_flags);
    /* Create a new zone that supports malloc_make{un}purgeable() discipline. */

extern malloc_zone_t *create_legacy_scalable_zone(size_t initial_size, unsigned debug_flags);
    /*
     * For use by CheckFix: create a new zone whose behavior is, apart from
     * the use of death-row and per-CPU magazines, that of Leopard.
     */

/*****	Private API for debug and performance tools	********/

extern boolean_t scalable_zone_statistics(malloc_zone_t *zone, malloc_statistics_t *stats, unsigned subzone);
    /* Fills stats with some statistics;
    1 is returned on success; else 0 is returned
    Currently: subzone=0 => tiny; subzone=1 => small; subzone=2 => large; subzone=3 => huge; any other subzone => returns 0 
    */

