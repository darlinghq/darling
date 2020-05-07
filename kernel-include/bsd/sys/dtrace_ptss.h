/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
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

#ifndef _DTRACE_PTSS_H_
#define _DTRACE_PTSS_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * The pid provider needs a small per thread scratch space,
 * in the address space of the user task. This code is used to
 * manage that space.
 *
 * High level design:
 *
 * To avoid serialization, this is a (mostly) lockless allocator. If
 * a new page has to be allocated, the process's sprlock will be acquired.
 *
 * NOTE: The dtrace copyin/copyout code is still the shared code that
 * can handle unmapped pages, so the scratch space isn't wired for now.
 * * Each page in user space is wired. It cannot be paged out, because
 * * dtrace's copyin/copyout is only guaranteed to handle pages already
 * * in memory.
 *
 * Each page in user space is represented by a dt_ptss_page. Page entries
 * are chained. Once allocated, a page is not freed until dtrace "cleans up"
 * that process.
 *
 * Clean up works like this:
 *
 * At process exit, free all kernel allocated memory, but ignore user pages.
 * At process exec, free all kernel allocated memory, but ignore user pages.
 * At process fork, free user pages copied from parent, and do not allocate kernel memory.
 *
 * This is making the assumption that it is faster to let the bulk vm_map
 * operations in exec/exit do their work, instead of explicit page free(s)
 * via mach_vm_deallocate.
 *
 * As each page is allocated, its entries are chained and added to the
 * free_list. To claim an entry, cas it off the list. When a thread exits,
 * cas its entry onto the list. We could potentially optimize this by
 * keeping a head/tail, and cas'ing the frees to the tail instead of the
 * head. Without evidence to support such a need, it seems better to keep
 * things simple for now.
 */


#define DTRACE_PTSS_SCRATCH_SPACE_PER_THREAD (64)

#define DTRACE_PTSS_ENTRIES_PER_PAGE (PAGE_MAX_SIZE / DTRACE_PTSS_SCRATCH_SPACE_PER_THREAD)

struct dtrace_ptss_page_entry {
	struct dtrace_ptss_page_entry*  next;
	user_addr_t                     addr;
	user_addr_t                     write_addr;
};

struct dtrace_ptss_page {
	struct dtrace_ptss_page*       next;
	struct dtrace_ptss_page_entry  entries[DTRACE_PTSS_ENTRIES_PER_PAGE];
};

struct dtrace_ptss_page_entry*  dtrace_ptss_claim_entry(struct proc* p); /* sprlock not held */
struct dtrace_ptss_page_entry*  dtrace_ptss_claim_entry_locked(struct proc* p); /* sprlock held */
void                            dtrace_ptss_release_entry(struct proc* p, struct dtrace_ptss_page_entry* e);

struct dtrace_ptss_page*        dtrace_ptss_allocate_page(struct proc* p);
void                            dtrace_ptss_free_page(struct proc* p, struct dtrace_ptss_page* ptss_page);

void                            dtrace_ptss_enable(struct proc* p);
void                            dtrace_ptss_exec_exit(struct proc* p);
void                            dtrace_ptss_fork(struct proc* parent, struct proc* child);

#ifdef  __cplusplus
}
#endif

#endif  /* _DTRACE_PTSS_H_ */
