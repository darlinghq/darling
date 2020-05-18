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

/*
 * For use by CheckFix: create a new zone whose behavior is, apart from
 * the use of death-row and per-CPU magazines, that of Leopard.
 */
static MALLOC_NOINLINE void *
legacy_valloc(szone_t *szone, size_t size)
{
	void *ptr;
	size_t num_kernel_pages;

	num_kernel_pages = round_page_quanta(size) >> vm_page_quanta_shift;
	ptr = large_malloc(szone, num_kernel_pages, 0, TRUE);
#if DEBUG_MALLOC
	if (LOG(szone, ptr)) {
		malloc_report(ASL_LEVEL_INFO, "legacy_valloc returned %p\n", ptr);
	}
#endif
	return ptr;
}

malloc_zone_t *
create_legacy_scalable_zone(size_t initial_size, unsigned debug_flags)
{
	malloc_zone_t *mzone = create_scalable_zone(initial_size, debug_flags);
	szone_t *szone = (szone_t *)mzone;

	if (!szone) {
		return NULL;
	}

	mprotect(szone, sizeof(szone->basic_zone), PROT_READ | PROT_WRITE);
	szone->basic_zone.valloc = (void *)legacy_valloc;
	szone->basic_zone.free_definite_size = NULL;
	mprotect(szone, sizeof(szone->basic_zone), PROT_READ);

	return mzone;
}
