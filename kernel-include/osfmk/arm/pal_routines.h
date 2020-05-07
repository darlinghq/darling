/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#ifndef _ARM_PAL_ROUTINES_H
#define _ARM_PAL_ROUTINES_H

#include <stdint.h>
#include <string.h>

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef XNU_KERNEL_PRIVATE

/* No-op */
#define pal_dbg_set_task_name( x ) do { } while(0)

#define pal_ast_check(thread)
#define pal_thread_terminate_self(t)

/* serial / debug output routines */
extern int  pal_serial_init(void);
extern void pal_serial_putc(char a);
extern void pal_serial_putc_nocr(char a);
extern int  pal_serial_getc(void);

#define panic_display_pal_info() do { } while(0)
#define pal_kernel_announce() do { } while(0)

#endif  /* XNU_KERNEL_PRIVATE */

/* Allows us to set a property on the IOResources object. Unused on ARM. */
static inline void
pal_get_resource_property(const char **property_name,
    int *property_value)
{
	*property_name = NULL;
	(void) property_value;
}

#if defined(__cplusplus)
}
#endif

#endif /* _ARM_PAL_ROUTINES_H */
