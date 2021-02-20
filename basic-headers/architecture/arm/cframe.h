/*
 * Copyright (c) 1999-2004, 2008 Apple Inc. All rights reserved.
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
/* Copyright (c) 1991 NeXT Software, Inc.  All rights reserved.
 *
 *	File:	architecture/ppc/cframe.h
 *	Author:	Mike DeMoney, NeXT Software, Inc.
 *
 *	This include file defines C calling sequence defines
 *	for ppc port.
 *
 * HISTORY
 * 20-May-97  Umesh Vaishampayan  (umeshv@apple.com)
 *	Added C_RED_ZONE.
 * 29-Dec-96  Umesh Vaishampayan  (umeshv@NeXT.com)
 *	Ported from m98k.
 * 11-June-91  Mike DeMoney (mike@next.com)
 *	Created.
 */

#ifndef	_ARCH_ARM_CFRAME_H_
#define	_ARCH_ARM_CFRAME_H_

/* Note that these values are copies of the somewhat more authoritative
 * values in <architecture/ppc/mode_independent_asm.h>.  We do not
 * include that file to avoid breaking legacy clients due to name
 * collisions.
 *
 * Note also that C_ARGSAVE_LEN isn't well defined or useful in PPC.
 * Most legacy uses of it are assuming it is the minimum stack frame
 * size, which is what we define it to be.
 */
#if defined(__arm__)
#define	C_ARGSAVE_LEN	32      /* "minimum arg save area" (but see above) */
#define	C_STACK_ALIGN	16      /* stack must be 16 byte aligned */
#define	C_RED_ZONE      224     /* 224 bytes to skip over saved registers */
#elif defined (__arm64__)
#define	C_STACK_ALIGN	16      /* stack must be 32 byte aligned */
#else /* !defined(__arm__) && !defined(__arm64__) */
#error Unknown architecture
#endif /* !defined(__arm__) && !defined(__arm64__) */

#endif	/* _ARCH_ARM_CFRAME_H_ */
