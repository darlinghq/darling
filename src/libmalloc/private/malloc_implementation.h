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

/* Private interfaces between libsystem_malloc, libSystem, and MallocStackLogging */

#include <malloc/malloc.h>

#ifndef _MALLOC_IMPLEMENTATION_H_
#define _MALLOC_IMPLEMENTATION_H_


/*********	Libsystem initializers ************/

struct _malloc_functions {
	unsigned long version;
	/* The following functions are included in version 1 of this structure */
	void * (*dlopen) (const char *path, int mode);
	void * (*dlsym) (void *handle, const char *symbol);
};

void __malloc_init(const char *apple[]);
void __stack_logging_early_finished(const struct _malloc_functions *);



/*
 * Definitions intended for the malloc stack logging library only.
 * This is SPI that is *not* intended for use elsewhere. It will change
 * and will eventually be removed, without prior warning.
 */
#if MALLOC_ENABLE_MSL_LITE_SPI

typedef struct szone_s szone_t;

typedef struct _malloc_msl_lite_hooks_s {
	szone_t *(*create_and_insert_msl_lite_zone)(const char *name,
											void *mallocp, void *callocp,
											void *vallocp, void *reallocp, void *batch_mallocp,
											void *batch_freep, void *memalignp, void *freep,
											void *free_definite_sizep, void *sizep);
	malloc_zone_t *(*helper_zone)(szone_t *zone);
	size_t (*szone_size)(szone_t *szone, const void *ptr);
	void *(*szone_malloc)(szone_t *szone, size_t size);
	void *(*szone_malloc_should_clear)(szone_t *szone, size_t size,
									   boolean_t cleared_requested);
	void (*szone_free)(szone_t *szone, void *ptr);
	void *(*szone_realloc)(szone_t *szone, void *ptr, size_t new_size);
	void *(*szone_valloc)(szone_t *szone, size_t size);
	void *(*szone_memalign)(szone_t *szone, size_t alignment, size_t size);
	unsigned (*szone_batch_malloc)(szone_t *szone, size_t size, void **results,
								   unsigned count);
	void (*szone_batch_free)(szone_t *szone, void **to_be_freed, unsigned count);
	boolean_t (*has_default_zone0)(void);
	
	size_t (*calloc_get_size)(size_t num_items, size_t size, size_t extra_size,
							  size_t *total_size);

	size_t (*szone_good_size)(szone_t *szone, size_t size);
	malloc_zone_t *(*basic_zone)(szone_t *zone);
} _malloc_msl_lite_hooks_t;

#endif // MALLOC_ENABLE_MSL_LITE_SPI

#endif

