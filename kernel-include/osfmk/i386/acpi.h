/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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

#ifndef _I386_ACPI_H_
#define _I386_ACPI_H_

/*
 * ACPI (Advanced Configuration and Power Interface) support.
 */

/*
 * Wake up code linear address. Wake and MP startup copy
 * code to this physical address and then jump to the
 * address started at PROT_MODE_START. Some small amount
 * below PROT_MODE_START is used as scratch space
 */
#define PROT_MODE_START 0x800
#define REAL_MODE_BOOTSTRAP_OFFSET 0x2000

#ifndef ASSEMBLER
typedef void (*acpi_sleep_callback)(void * refcon);
extern vm_offset_t acpi_install_wake_handler(void);
extern void        acpi_sleep_kernel(acpi_sleep_callback func, void * refcon);
extern void        acpi_idle_kernel(acpi_sleep_callback func, void * refcon);
void install_real_mode_bootstrap(void *prot_entry);
#endif  /* ASSEMBLER */

#endif /* !_I386_ACPI_H_ */
