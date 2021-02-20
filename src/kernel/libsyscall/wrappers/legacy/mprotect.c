/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#ifndef NO_SYSCALL_LEGACY

#define _NONSTD_SOURCE
#include <sys/cdefs.h>

#include "_errno.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <mach/vm_param.h>

/*
 * Stub function to account for the differences in standard compliance
 * while maintaining binary backward compatibility.
 *
 * This is only the legacy behavior.
 */
extern int __mprotect(void *, size_t, int);

int
mprotect(void *addr, size_t len, int prot)
{
	void    *aligned_addr;
	size_t  offset;
	int     rv;

	/*
	 * Page-align "addr" since the system now requires it
	 * for standards compliance.
	 * Update "len" to reflect the alignment.
	 */
	offset = ((uintptr_t) addr) & PAGE_MASK;
	aligned_addr = (void *) (((uintptr_t) addr) & ~PAGE_MASK);
	len += offset;
	rv = __mprotect(aligned_addr, len, prot);
	if (rv == -1 && errno == ENOMEM) {
		/*
		 * Standards now require that we return ENOMEM if there was
		 * a hole in the address range.  Panther and earlier used
		 * to return an EINVAL error, so honor backwards compatibility.
		 */
		errno = EINVAL;
	}
	return rv;
}

#endif /* NO_SYSCALL_LEGACY */
