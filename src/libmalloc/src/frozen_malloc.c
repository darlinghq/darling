/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/********* Support code for emacs unexec ************/

/* History of freezedry version numbers:
 *
 * 1) Old malloc (before the scalable malloc implementation in this file
 *    existed).
 * 2) Original freezedrying code for scalable malloc.  This code was apparently
 *    based on the old freezedrying code and was fundamentally flawed in its
 *    assumption that tracking allocated memory regions was adequate to fake
 *    operations on freezedried memory.  This doesn't work, since scalable
 *    malloc does not store flags in front of large page-aligned allocations.
 * 3) Original szone-based freezedrying code.
 * 4) Fresher malloc with tiny zone
 * 5) 32/64bit compatible malloc
 * 6) Metadata within 1MB and 8MB region for tiny and small
 *
 * No version backward compatibility is provided, but the version number does
 * make it possible for malloc_jumpstart() to return an error if the application
 * was freezedried with an older version of malloc.
 */
#define MALLOC_FREEZEDRY_VERSION 6

typedef struct {
	unsigned version;
	unsigned nszones;
	szone_t *szones;
} malloc_frozen;

static void *
frozen_malloc(szone_t *zone, size_t new_size)
{
	return malloc(new_size);
}

static void *
frozen_calloc(szone_t *zone, size_t num_items, size_t size)
{
	return calloc(num_items, size);
}

static void *
frozen_valloc(szone_t *zone, size_t new_size)
{
	return valloc(new_size);
}

static void *
frozen_realloc(szone_t *zone, void *ptr, size_t new_size)
{
	size_t old_size = szone_size(zone, ptr);
	void *new_ptr;

	if (new_size <= old_size) {
		return ptr;
	}
	new_ptr = malloc(new_size);
	if (old_size > 0) {
		memcpy(new_ptr, ptr, old_size);
	}
	return new_ptr;
}

static void
frozen_free(szone_t *zone, void *ptr)
{
}

static void
frozen_destroy(szone_t *zone)
{
}

/********* Pseudo-private API for emacs unexec ************/

/*
 * malloc_freezedry() records all of the szones in use, so that they can be
 * partially reconstituted by malloc_jumpstart().  Due to the differences
 * between reconstituted memory regions and those created by the szone code,
 * care is taken not to reallocate from the freezedried memory, except in the
 * case of a non-growing realloc().
 *
 * Due to the flexibility provided by the zone registration mechanism, it is
 * impossible to implement generic freezedrying for any zone type.  This code
 * only handles applications that use the szone allocator, so malloc_freezedry()
 * returns 0 (error) if any non-szone zones are encountered.
 */

uintptr_t
malloc_freezedry(void)
{
	extern unsigned malloc_num_zones;
	extern malloc_zone_t **malloc_zones;
	malloc_frozen *data;
	unsigned i;

	/* Allocate space in which to store the freezedry state. */
	data = (malloc_frozen *)malloc(sizeof(malloc_frozen));

	/* Set freezedry version number so that malloc_jumpstart() can check for compatibility. */
	data->version = MALLOC_FREEZEDRY_VERSION;

	/* Allocate the array of szone pointers. */
	data->nszones = malloc_num_zones;
	data->szones = (szone_t *)calloc(malloc_num_zones, sizeof(szone_t));

	/*
	 * Fill in the array of szone structures.  They are copied rather than
	 * referenced, since the originals are likely to be clobbered during malloc
	 * initialization.
	 */
	for (i = 0; i < malloc_num_zones; i++) {
		if (strcmp(malloc_zones[i]->zone_name, "DefaultMallocZone")) {
			/* Unknown zone type. */
			free(data->szones);
			free(data);
			return 0;
		}
		memcpy(&data->szones[i], malloc_zones[i], sizeof(szone_t));
	}

	return ((uintptr_t)data);
}

int
malloc_jumpstart(uintptr_t cookie)
{
	malloc_frozen *data = (malloc_frozen *)cookie;
	unsigned i;

	if (data->version != MALLOC_FREEZEDRY_VERSION) {
		/* Unsupported freezedry version. */
		return 1;
	}

	for (i = 0; i < data->nszones; i++) {
		/* Set function pointers.  Even the functions that stay the same must be
		 * set, since there are no guarantees that they will be mapped to the
		 * same addresses. */
		data->szones[i].basic_zone.size = (void *)szone_size;
		data->szones[i].basic_zone.malloc = (void *)frozen_malloc;
		data->szones[i].basic_zone.calloc = (void *)frozen_calloc;
		data->szones[i].basic_zone.valloc = (void *)frozen_valloc;
		data->szones[i].basic_zone.free = (void *)frozen_free;
		data->szones[i].basic_zone.realloc = (void *)frozen_realloc;
		data->szones[i].basic_zone.destroy = (void *)frozen_destroy;
		data->szones[i].basic_zone.introspect = (struct malloc_introspection_t *)&szone_introspect;

		/* Register the freezedried zone. */
		malloc_zone_register(&data->szones[i].basic_zone);
	}

	return 0;
}
