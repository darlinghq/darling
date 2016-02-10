/*
 * Copyright (c) 2000-2014 Apple Inc. All rights reserved.
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
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)vnode.h	8.17 (Berkeley) 5/20/95
 */
 
#ifndef _VNODE_H_
#define _VNODE_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

/*
 * The vnode is the focus of all file activity in UNIX.  There is a
 * unique vnode allocated for each active file, each current directory,
 * each mounted-on file, text file, and the root.
 */

/*
 * Vnode types.  VNON means no type.
 */
enum vtype	{ 
	/* 0 */
	VNON, 
	/* 1 - 5 */
	VREG, VDIR, VBLK, VCHR, VLNK, 
	/* 6 - 10 */
	VSOCK, VFIFO, VBAD, VSTR, VCPLX 
};

/*
 * Vnode tag types.
 * These are for the benefit of external programs only (e.g., pstat)
 * and should NEVER be inspected by the kernel.
 */
enum vtagtype	{
	/* 0 */
	VT_NON,
	/* 1 reserved, overlaps with (CTL_VFS, VFS_NUMMNTOPS) */
	VT_UFS,
	/* 2 - 5 */
	VT_NFS, VT_MFS, VT_MSDOSFS, VT_LFS,
	/* 6 - 10 */
	VT_LOFS, VT_FDESC, VT_PORTAL, VT_NULL, VT_UMAP, 
	/* 11 - 15 */
	VT_KERNFS, VT_PROCFS, VT_AFS, VT_ISOFS, VT_MOCKFS,
	/* 16 - 20 */
	VT_HFS, VT_ZFS, VT_DEVFS, VT_WEBDAV, VT_UDF, 
	/* 21 - 24 */
	VT_AFP, VT_CDDA, VT_CIFS, VT_OTHER
};


/*
 * flags for VNOP_BLOCKMAP
 */
#define VNODE_READ	0x01
#define VNODE_WRITE	0x02



/* flags for VNOP_ALLOCATE */
#define	PREALLOCATE		0x00000001	/* preallocate allocation blocks */
#define	ALLOCATECONTIG	0x00000002	/* allocate contigious space */
#define	ALLOCATEALL		0x00000004	/* allocate all requested space */
									/* or no space at all */
#define	FREEREMAINDER	0x00000008	/* deallocate allocated but */
									/* unfilled blocks */
#define	ALLOCATEFROMPEOF	0x00000010	/* allocate from the physical eof */
#define	ALLOCATEFROMVOL		0x00000020	/* allocate from the volume offset */

/*
 * Token indicating no attribute value yet assigned. some user source uses this
 */
#define	VNOVAL	(-1)



#endif /* !_VNODE_H_ */
