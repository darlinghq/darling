/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef _KERN_SFI_H_
#define _KERN_SFI_H_

#include <stdint.h>
#include <mach/mach_types.h>
#include <mach/kern_return.h>
#include <mach/sfi_class.h>
#include <kern/ast.h>
#include <kern/kern_types.h>
#include <kern/ledger.h>

extern void sfi_init(void);
extern void sfi_early_init(void);
extern sfi_class_id_t sfi_get_ledger_alias_for_class(sfi_class_id_t class_id);
extern int sfi_ledger_entry_add(ledger_template_t template, sfi_class_id_t class_id);

kern_return_t sfi_set_window(uint64_t window_usecs);
kern_return_t sfi_window_cancel(void);
kern_return_t sfi_get_window(uint64_t *window_usecs);

kern_return_t sfi_set_class_offtime(sfi_class_id_t class_id, uint64_t offtime_usecs);
kern_return_t sfi_class_offtime_cancel(sfi_class_id_t class_id);
kern_return_t sfi_get_class_offtime(sfi_class_id_t class_id, uint64_t *offtime_usecs);

#ifdef MACH_KERNEL_PRIVATE
/*
 * Classifying a thread requires no special locks to be held (although attribute
 * changes that cause an inconsistent snapshot may cause a spurious AST). Final
 * evaluation will happen at the AST boundary with the thread locked. If possible,
 *
 */
sfi_class_id_t sfi_thread_classify(thread_t thread);
sfi_class_id_t sfi_processor_active_thread_classify(processor_t processor);
ast_t sfi_thread_needs_ast(thread_t thread, sfi_class_id_t *out_class /* optional */);
ast_t sfi_processor_needs_ast(processor_t processor);

void sfi_ast(thread_t thread);
void sfi_reevaluate(thread_t thread);
kern_return_t sfi_defer(uint64_t);
#endif /* MACH_KERNEL_PRIVATE */

#endif /* _KERN_SFI_H_ */
