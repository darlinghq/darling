/*
 * Copyright (c) 2005-2006 Apple Computer, Inc. All rights reserved.
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

#ifndef _FBT_H
#define _FBT_H

#if defined(__i386__) || defined (__x86_64__)
typedef uint8_t machine_inst_t;
#else
#error Unknown Architecture
#endif

#define MAX_FBTP_NAME_CHARS 16

typedef struct fbt_probe {
	struct fbt_probe *fbtp_hashnext;
	machine_inst_t	*fbtp_patchpoint;
	int8_t			fbtp_rval;
	machine_inst_t	fbtp_patchval;
	machine_inst_t	fbtp_savedval;
        machine_inst_t  fbtp_currentval;
	uintptr_t		fbtp_roffset;
	dtrace_id_t		fbtp_id;
	/* FIXME!
	 * This field appears to only be used in error messages.
	 * It puts this structure into the next size bucket in kmem_alloc
	 * wasting 32 bytes per probe. (in i386 only)
	 */
	char			fbtp_name[MAX_FBTP_NAME_CHARS];
	struct modctl	*fbtp_ctl;
	int		fbtp_loadcnt;
#if !defined(__APPLE__)
	int		fbtp_symndx;
#endif
	struct fbt_probe *fbtp_next;
} fbt_probe_t;

extern int dtrace_invop(uintptr_t, uintptr_t *, uintptr_t);
extern int fbt_invop(uintptr_t, uintptr_t *, uintptr_t);
extern void fbt_provide_module(void *, struct modctl *);
extern int fbt_enable (void *arg, dtrace_id_t id, void *parg);
#endif /* _FBT_H */
