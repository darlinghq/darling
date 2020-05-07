/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef SYS_DOC_TOMBSTONE_H_
#define SYS_DOC_TOMBSTONE_H_

#include <sys/types.h>
#include <stdbool.h>

#ifdef KERNEL_PRIVATE

/*
 * struct representing a document "tombstone" that's recorded
 * when a thread manipulates files marked with a document-id.
 * if the thread recreates the same item, this tombstone is
 * used to preserve the document_id on the new file.
 *
 * It is a separate structure because of its size - we want to
 * allocate it on demand instead of just stuffing it into the
 * uthread structure.
 */
struct doc_tombstone {
	struct vnode     *t_lastop_parent;
	struct vnode     *t_lastop_item;
	uint32_t                  t_lastop_parent_vid;
	uint32_t                  t_lastop_item_vid;
	uint64_t          t_lastop_fileid;
	uint64_t          t_lastop_document_id;
	unsigned char     t_lastop_filename[NAME_MAX + 1];
};

struct componentname;

struct doc_tombstone *doc_tombstone_get(void);
void doc_tombstone_clear(struct doc_tombstone *ut, struct vnode **old_vpp);
void doc_tombstone_save(struct vnode *dvp, struct vnode *vp,
    struct componentname *cnp, uint64_t doc_id,
    ino64_t file_id);
bool doc_tombstone_should_ignore_name(const char *nameptr, int len);
bool doc_tombstone_should_save(struct doc_tombstone *ut, struct vnode *vp,
    struct componentname *cnp);

#endif // defined(KERNEL_PRIVATE)

#endif // SYS_DOC_TOMBSTONE_H_
