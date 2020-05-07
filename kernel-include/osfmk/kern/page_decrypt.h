/*
 * Copyright (c) 2005-2008 Apple Inc. All rights reserved.
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

#ifdef  KERNEL_PRIVATE

#ifndef _KERN_PAGE_DECRYPT_H
#define _KERN_PAGE_DECRYPT_H

#include <mach/machine.h>

/*
 * Interface for DSMOS
 */
typedef int     (*dsmos_page_transform_hook_t) (const void *, void*, unsigned long long, void *);
extern  void    dsmos_page_transform_hook(dsmos_page_transform_hook_t hook);    /* exported */

extern  int     dsmos_page_transform(const void *, void*, unsigned long long, void*);


/*
 * Interface for text decryption family
 */
struct pager_crypt_info {
	/* Decrypt one page */
	int     (*page_decrypt)(const void *src_vaddr, void *dst_vaddr,
	    unsigned long long src_offset, void *crypt_ops);
	/* Pager using this crypter terminates - crypt module not needed anymore */
	void    (*crypt_end)(void *crypt_ops);
	/* Private data for the crypter */
	void    *crypt_ops;
	volatile int    crypt_refcnt;
};
typedef struct pager_crypt_info pager_crypt_info_t;

struct crypt_file_data {
	char          *filename;
	cpu_type_t     cputype;
	cpu_subtype_t  cpusubtype;
};
typedef struct crypt_file_data crypt_file_data_t;

typedef int (*text_crypter_create_hook_t)(struct pager_crypt_info *crypt_info,
    const char *id, void *crypt_data);
extern void text_crypter_create_hook_set(text_crypter_create_hook_t hook);
extern text_crypter_create_hook_t text_crypter_create;

#endif  /* _KERN_PAGE_DECRYPT_H */

#endif  /* KERNEL_PRIVATE */
