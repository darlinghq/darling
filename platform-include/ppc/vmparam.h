/*
 * Copyright (c) 2000-2002 Apple Computer, Inc. All rights reserved.
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

#ifndef	_BSD_PPC_VMPARAM_H_
#define	_BSD_PPC_VMPARAM_H_ 1

#include <sys/resource.h>

#define	USRSTACK	(0xc0000000)

/* 
 * put the default 64-bit stack at the max address
 * (minus one 32-bit address space for other incidentals)
 */
#define	USRSTACK64 (0x00007FFF5FC00000ULL)

/*
 * Virtual memory related constants, all in bytes
 */
#ifndef DFLDSIZ
#define	DFLDSIZ		(RLIM_INFINITY)		/* initial data size limit */
// XXX Not enforced
//#define DFLDSIZ	(6*1024*1024)		/* initial data size limit */
#endif
#ifndef MAXDSIZ
#define	MAXDSIZ		(RLIM_INFINITY)		/* max data size */
#endif
#ifndef	DFLSSIZ
#define	DFLSSIZ		(8*1024*1024)		/* initial stack size limit */
#endif
#ifndef	MAXSSIZ
#define	MAXSSIZ		(64*1024*1024)		/* max stack size */
#endif
#ifndef	DFLCSIZ
#define DFLCSIZ		(0)			/* initial core size limit */
#endif
#ifndef	MAXCSIZ
#define MAXCSIZ		(RLIM_INFINITY)		/* max core size */
#endif

#endif	/* _BSD_PPC_VMPARAM_H_ */
