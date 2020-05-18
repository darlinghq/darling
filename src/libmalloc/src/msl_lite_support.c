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

/*
 * Support for stack logging lite in the malloc stack logging library. This code
 * will go away when MSL lite no longer requires its own zone.
 */
#ifndef MALLOC_ENABLE_MSL_LITE_SPI
#define MALLOC_ENABLE_MSL_LITE_SPI 1
#endif // MALLOC_ENABLE_MSL_LITE_SPI

#include "internal.h"

// These definitions are here and not in a header file because all of this code
// is intended to go away very soon.
extern malloc_zone_t **malloc_zones;
extern malloc_zone_t* lite_zone;
extern void malloc_zone_register_while_locked(malloc_zone_t *zone);
extern boolean_t has_default_zone0(void);

static szone_t *create_and_insert_msl_lite_zone(const char *name,
		void *mallocp, void *callocp,
		void *vallocp, void *reallocp, void *batch_mallocp,
		void *batch_freep, void *memalignp, void *freep,
		void *free_definite_sizep, void *sizep);

static size_t
_calloc_get_size(size_t num_items, size_t size, size_t extra_size,
		size_t *total_size)
{
	// calloc_get_size is inlined.
	return calloc_get_size(num_items, size, extra_size, total_size);
}

static malloc_zone_t *
szone_helper_zone(szone_t *zone)
{
	return &zone->helper_zone->basic_zone;
}

static malloc_zone_t *
szone_basic_zone(szone_t *zone)
{
	return &zone->basic_zone;
}

static struct _malloc_msl_lite_hooks_s malloc_msl_lite_hooks = {
	.create_and_insert_msl_lite_zone = &create_and_insert_msl_lite_zone,
	.helper_zone = &szone_helper_zone,
	.szone_size = &szone_size,
	.szone_malloc = &szone_malloc,
	.szone_malloc_should_clear = &szone_malloc_should_clear,
	.szone_free = &szone_free,
	.szone_realloc = &szone_realloc,
	.szone_valloc = &szone_valloc,
	.szone_memalign = &szone_memalign,
	.szone_batch_malloc = &szone_batch_malloc,
	.szone_batch_free = &szone_batch_free,
	.has_default_zone0 = &has_default_zone0,
	.calloc_get_size = &_calloc_get_size,
	.basic_zone = &szone_basic_zone,
	.szone_good_size = &szone_good_size,
};

static szone_t *
create_and_insert_msl_lite_zone(const char *name,
		void *mallocp, void *callocp,
		void *vallocp, void *reallocp, void *batch_mallocp,
		void *batch_freep, void *memalignp, void *freep,
		void *free_definite_sizep, void *sizep)
{
// TODO: this has to be locked in some way....
	szone_t* szone = create_scalable_szone(0, malloc_debug_flags);
	malloc_zone_t *zone = &szone->basic_zone;
	
	// unprotect function pointers
	mprotect(szone, sizeof(szone->basic_zone), PROT_READ | PROT_WRITE);
	
	// set the function pointers
	szone->basic_zone.malloc = mallocp;
	szone->basic_zone.calloc = callocp;
	szone->basic_zone.valloc = vallocp;
	szone->basic_zone.realloc = reallocp;
	szone->basic_zone.batch_malloc = batch_mallocp;
	szone->basic_zone.batch_free = batch_freep;
	szone->basic_zone.memalign = memalignp;
	szone->basic_zone.free = freep;
	szone->basic_zone.free_definite_size = free_definite_sizep;
	szone->basic_zone.size = sizep;
	
	// protect function pointers
	mprotect(szone, sizeof(szone->basic_zone), PROT_READ);
	
	// set helper zone
	szone->helper_zone = (szone_t *)malloc_zones[0];
	
	malloc_zone_register_while_locked(zone);
	malloc_set_zone_name(zone, name);
	lite_zone = zone;

	return szone;
}

/*
 * Copies the malloc library's _malloc_msl_lite_hooks_t structure to a given
 * location. We pass the structure size to allow the structure to
 * grow. Since this is a temporary arrangement, we don't need to worry about
 * pointer authentication here or in the _malloc_msl_lite_hooks_t structure itself.
 */
MALLOC_NOEXPORT
void
set_msl_lite_hooks(set_msl_lite_hooks_callout_t callout)
{
	callout(&malloc_msl_lite_hooks, sizeof(malloc_msl_lite_hooks));
}
