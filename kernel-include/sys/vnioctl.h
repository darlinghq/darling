/*
 * Copyright (c) 2004-2006 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * from: Utah $Hdr: fdioctl.h 1.1 90/07/09$
 *
 *	@(#)vnioctl.h	8.1 (Berkeley) 6/10/93
 *
 * $FreeBSD: src/sys/sys/vnioctl.h,v 1.4 1999/09/17 05:33:58 dillon Exp $
 */

#ifndef _SYS_VNIOCTL_H_
#define _SYS_VNIOCTL_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>


/*
 * Ioctl definitions for file (vnode) disk pseudo-device.
 */

#define _PATH_VNTAB	"/etc/vntab"	/* default config file */

typedef enum {
	vncontrol_readwrite_io_e = 0
} vncontrol_t;

struct vn_ioctl {
	char *		vn_file;	/* pathname of file to mount */
	int			vn_size;	/* (returned) size of disk */
	vncontrol_t	vn_control;
};

#ifdef KERNEL_PRIVATE

struct vn_ioctl_64 {
	u_int64_t	vn_file;	/* pathname of file to mount */
	int			vn_size;	/* (returned) size of disk */
	vncontrol_t	vn_control;
};

struct vn_ioctl_32 {
	u_int32_t	vn_file;	/* pathname of file to mount */
	int			vn_size;	/* (returned) size of disk */
	vncontrol_t	vn_control;
};

#endif /* KERNEL_PRIVATE */

/*
 * Before you can use a unit, it must be configured with VNIOCSET.
 * The configuration persists across opens and closes of the device;
 * an VNIOCCLR must be used to reset a configuration.  An attempt to
 * VNIOCSET an already active unit will return EBUSY.
 */
#define VNIOCATTACH	_IOWR('F', 0, struct vn_ioctl)	/* attach file */
#define VNIOCDETACH	_IOWR('F', 1, struct vn_ioctl)	/* detach disk */
#define VNIOCGSET	_IOWR('F', 2, u_int32_t )		/* set global option */
#define VNIOCGCLEAR	_IOWR('F', 3, u_int32_t )		/* reset --//-- */
#define VNIOCUSET	_IOWR('F', 4, u_int32_t )		/* set unit option */
#define VNIOCUCLEAR	_IOWR('F', 5, u_int32_t )		/* reset --//-- */
#define VNIOCSHADOW	_IOWR('F', 6, struct vn_ioctl)	/* attach shadow */
#ifdef KERNEL_PRIVATE
#define VNIOCATTACH64	_IOWR('F', 0, struct vn_ioctl_64)	/* attach file - LP64 */
#define VNIOCDETACH64	_IOWR('F', 1, struct vn_ioctl_64)	/* detach disk - LP64 */
#define VNIOCSHADOW64	_IOWR('F', 6, struct vn_ioctl_64)	/* attach shadow - LP64 */
#ifdef __LP64__
#define VNIOCATTACH32	_IOWR('F', 0, struct vn_ioctl_32)	/* attach file - U32 version for K64 */
#define VNIOCDETACH32	_IOWR('F', 1, struct vn_ioctl_32)	/* detach disk - U32 version for K64 */
#define VNIOCSHADOW32	_IOWR('F', 6, struct vn_ioctl_32)	/* attach shadow - U32 version for K64 */
#endif
#endif /* KERNEL_PRIVATE */

#define VN_LABELS	0x1	/* Use disk(/slice) labels */
#define VN_FOLLOW	0x2	/* Debug flow in vn driver */
#define VN_DEBUG	0x4	/* Debug data in vn driver */
#define VN_IO		0x8	/* Debug I/O in vn driver */
#define VN_DONTCLUSTER	0x10	/* Don't cluster */
#define VN_RESERVE	0x20	/* Pre-reserve swap */


#endif	/* _SYS_VNIOCTL_H_*/
