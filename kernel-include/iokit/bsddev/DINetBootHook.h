/*
 * Copyright (c) 2002-2016 Apple Inc. All rights reserved.
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

/*
 *  DINetBootHook.h
 *  DiskImages
 *
 *	Revision History
 *
 *	$Log: DINetBootHook.h,v $
 *	Revision 1.4  2005/07/29 21:49:57  lindak
 *	Merge of branch "chardonnay" to pick up all chardonnay changes in Leopard
 *	as of xnu-792.7.4
 *
 *	Revision 1.3.1582.1  2005/06/24 01:47:25  lindak
 *	Bringing over all of the Karma changes into chardonnay.
 *
 *	Revision 1.1.1.1  2005/02/24 21:48:06  akosut
 *	Import xnu-764 from Tiger8A395
 *
 *	Revision 1.3  2002/05/22 18:50:49  aramesh
 *	Kernel API Cleanup
 *	Bug #: 2853781
 *	Changes from Josh(networking), Rick(IOKit), Jim & David(osfmk), Umesh, Dan & Ramesh(BSD)
 *	Submitted by: Ramesh
 *	Reviewed by: Vincent
 *
 *	Revision 1.2.12.1  2002/05/21 23:08:14  aramesh
 *	Kernel API Cleanup
 *	Bug #: 2853781
 *	Submitted by: Josh, Umesh, Jim, Rick and Ramesh
 *	Reviewed by: Vincent
 *
 *	Revision 1.2  2002/05/03 18:08:39  lindak
 *	Merged PR-2909558 into Jaguar (siegmund POST WWDC: add support for NetBoot
 *	over IOHDIXController)
 *
 *	Revision 1.1.2.1  2002/04/24 22:29:12  dieter
 *	Bug #: 2909558
 *	- added IOHDIXController netboot stubs
 *
 *	Revision 1.2  2002/04/14 22:56:47  han
 *	fixed up comment re dev_t
 *
 *	Revision 1.1  2002/04/13 19:22:28  han
 *	added stub file DINetBookHook.c
 *
 *
 */

#ifndef __DINETBOOKHOOK_H__
#define __DINETBOOKHOOK_H__

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

/*
 *       Name:		di_root_image
 *       Function:	mount the disk image returning the dev node
 *       Parameters:	path	->		path/url to disk image
 *                               devname	<-		dev node used to set the rootdevice global variable
 *                               dev_p	<-		combination of major/minor node
 *       Comments:
 */
int di_root_image(const char *path, char *devname, size_t devsz, dev_t *dev_p);
void di_root_ramfile( IORegistryEntry * entry );
int di_root_ramfile_buf(void *buf, size_t bufsz, char *devname, size_t devsz, dev_t *dev_p);

#ifdef __cplusplus
};
#endif

#endif /* __APPLE_API_PRIVATE */

#endif /* __DINETBOOKHOOK_H__ */
