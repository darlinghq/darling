/*
 * Copyright (c) 2004, 2006 Apple Computer, Inc. All rights reserved.
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
#include <stdarg.h>

int __FCNTL(int, int, void *);

/*
 * Stub function to account for the differences in the size of the third
 * argument when int and void * are different sizes. Also add pthread
 * cancelability.
 */
int
fcntl(int fd, int cmd, ...)
{
	va_list ap;
	void *arg;

	va_start(ap, cmd);
	switch (cmd) {
	case F_GETLK:
	case F_GETLKPID:
	case F_SETLK:
	case F_SETLKW:
	case F_SETLKWTIMEOUT:
	case F_OFD_GETLK:
	case F_OFD_GETLKPID:
	case F_OFD_SETLK:
	case F_OFD_SETLKW:
	case F_OFD_SETLKWTIMEOUT:
	case F_PREALLOCATE:
	case F_PUNCHHOLE:
	case F_SETSIZE:
	case F_RDADVISE:
	case F_LOG2PHYS:
	case F_LOG2PHYS_EXT:
	case F_GETPATH:
	case F_GETPATH_NOFIRMLINK:
	case F_GETPATH_MTMINFO:
	case F_GETCODEDIR:
	case F_PATHPKG_CHECK:
	case F_OPENFROM:
	case F_UNLINKFROM:
	case F_ADDSIGS:
	case F_ADDFILESIGS:
	case F_ADDFILESIGS_FOR_DYLD_SIM:
	case F_ADDFILESIGS_RETURN:
	case F_FINDSIGS:
	case F_TRANSCODEKEY:
	case F_TRIM_ACTIVE_FILE:
	case F_SPECULATIVE_READ:
	case F_CHECK_LV:
		arg = va_arg(ap, void *);
		break;
	default:
		arg = (void *)((unsigned long)va_arg(ap, int));
		break;
	}
	va_end(ap);
	return __FCNTL(fd, cmd, arg);
}
