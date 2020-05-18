/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

/* Make TAPI happy by declaring things that other projects forward-declare. */
/* This header is not installed anywhere. */

/* For Libsystem */
void _malloc_fork_child(void);
void _malloc_fork_parent(void);
void _malloc_fork_prepare(void);

/* For various debugging tools? */

void scalable_zone_info(malloc_zone_t *zone, unsigned *info_to_fill, unsigned count);
void (*malloc_error(void (*func)(int)))(int);
extern uint64_t __mach_stack_logging_shared_memory_address;

/* At least for malloc_replay.cpp */
void mag_set_thread_index(unsigned int index);

/* Externally visible from magazine_malloc.h. Not worth pulling the whole header into tapi just for this one. */

boolean_t scalable_zone_statistics(malloc_zone_t *zone, malloc_statistics_t *stats, unsigned subzone);

/* Globals for performance tools, replicated here from the top of malloc.c */

typedef void(malloc_logger_t)(uint32_t type,
		uintptr_t arg1,
		uintptr_t arg2,
		uintptr_t arg3,
		uintptr_t result,
		uint32_t num_hot_frames_to_skip);
extern int32_t malloc_num_zones;
extern int32_t malloc_num_zones_allocated;
extern malloc_zone_t **malloc_zones;
extern malloc_logger_t *malloc_logger;
extern unsigned malloc_check_start;
extern unsigned malloc_check_counter;
extern unsigned malloc_check_each;
extern int _malloc_no_asl_log;
extern int _os_cpu_number_override;

/* Globally visible for manual debugging? */

extern unsigned szone_check_counter;
extern unsigned szone_check_start;
extern unsigned szone_check_modulo;

/* CoreServices checkfixes */
void malloc_create_legacy_default_zone(void);
void zeroify_scalable_zone(malloc_zone_t *zone);

/* This is extern-declared by some projects, like racoon (ipsec) */
/* Maybe we can change it to a symbol-alias of free? */
void vfree(void *ptr);

/* Obsolete entry points. They don't work, don't use them. */
void set_malloc_singlethreaded(boolean_t);
void malloc_singlethreaded(void);
int malloc_debug(int);

/* WeChat references this, only god knows why.  This symbol does nothing. */
extern int stack_logging_enable_logging;

/* For debugging */
void tiny_print_region_free_list(void *ptr, unsigned int slot);

