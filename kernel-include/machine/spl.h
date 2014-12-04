/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
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
#ifndef _BSD_MACHINE_SPL_H_
#define _BSD_MACHINE_SPL_H_

#ifdef KERNEL
#ifndef __ASSEMBLER__

#if !defined(__LP64__) || defined(XNU_KERNEL_PRIVATE)
/*
 *	Machine-dependent SPL definitions.
 *
 */
typedef unsigned	spl_t;

extern unsigned	int sploff(void);
extern unsigned	int splhigh(void);
extern unsigned	int splsched(void);
extern unsigned	int splclock(void);
extern unsigned	int splpower(void);
extern unsigned	int splvm(void);
extern unsigned	int splbio(void);
extern unsigned	int splimp(void);
extern unsigned	int spltty(void);
extern unsigned	int splnet(void);
extern unsigned	int splsoftclock(void);

extern void	spllo(void);
extern void	spl0(void);
extern void	splon(unsigned int level);
extern void	splx(unsigned int level);
extern void	spln(unsigned int level);
#define splstatclock()	splhigh()

#endif  /* !__LP64__ || XNU_KERNEL_PRIVATE */

#endif /* __ASSEMBLER__ */

#endif /* KERNEL */


#endif /* _BSD_MACHINE_SPL_H_ */
