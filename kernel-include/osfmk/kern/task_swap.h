/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 *
 * Revision 1.1.1.1  1998/09/22 21:05:32  wsanchez
 * Import of Mac OS X kernel (~semeria)
 *
 * Revision 1.1.1.1  1998/03/07 02:25:56  wsanchez
 * Import of OSF Mach kernel (~mburg)
 *
 * Revision 1.1.4.1  1995/04/07  19:02:38  barbou
 *      Merged into mainline.
 *      [95/03/09            barbou]
 *
 * Revision 1.1.2.2  1995/02/13  15:35:45  barbou
 *      Merged/ported to MK6.
 *
 * Revision 1.1.1.3  94/08/12  15:44:39  barbou
 *      VM Merge - Task Swapper.
 *
 *      Changed host_priv_t into host_t.
 *      [94/07/28            barbou]
 *
 * Revision 1.1.1.2  1994/07/28  15:33:46  barbou
 *      Copied from IK.
 *
 * Revision 3.0.3.2  1994/01/20  19:53:01  chasb
 *      Remove excessively restrictive copyright notice
 *      [1994/01/20  17:50:40  chasb]
 *
 * Revision 3.0.3.1  1993/12/20  21:06:49  gupta
 *      Expanded C O P Y R I G H T
 *      [1993/12/17  22:19:22  gupta]
 *
 * Revision 3.0  1992/12/31  22:08:24  ede
 *      Initial revision for OSF/1 R1.3
 *
 * Revision 1.1.4.5  1992/03/16  18:02:52  gmf
 *      Add TASK_SW_ELIGIBLE flag to swap_flags; prototype
 *      task_swapout_eligible, task_swapout_ineligible.
 *      [1992/02/12  22:01:48  gmf]
 *
 * Revision 1.1.4.4  1992/01/22  22:14:13  gmf
 *      Change prototype for task_swappable() to use host_priv_t
 *      instead of host_t.
 *      [1992/01/17  17:48:13  gmf]
 *
 * Revision 1.1.4.3  1991/12/10  17:20:55  gmf
 *      Add extern declaration for new thread.
 *      Changed TASK_SW_WAIT flag to TASK_SW_WANT_IN.
 *      [1991/12/10  16:19:10  gmf]
 *
 * Revision 1.1.4.2  1991/11/21  21:48:35  mmp
 *      initial task swapping code
 *      [1991/11/21  21:01:37  mmp]
 *
 * $EndLog$
 */

/*
 *	        File:	kern/task_swap.h
 *
 *	Task residency management primitives declarations.
 */

#ifndef _KERN_TASK_SWAP_H_
#define _KERN_TASK_SWAP_H_

#include <kern/host.h>

/*
 *	swap states
 */
#define TASK_SW_UNSWAPPABLE     1       /* not swappable */
#define TASK_SW_IN              2       /* swapped in (resident) */
#define TASK_SW_OUT             3       /* swapped out (non-resident) */
#define TASK_SW_COMING_IN       4       /* about to be swapped in */
#define TASK_SW_GOING_OUT       5       /* being swapped out */

/*
 *	swap flags
 */
#define TASK_SW_MAKE_UNSWAPPABLE        0x01    /* make it unswappable */
#define TASK_SW_WANT_IN                 0x02    /* sleeping on state */
#define TASK_SW_ELIGIBLE                0x04    /* eligible for swapping */

/*
 * exported routines
 */
extern void task_swapper_init(void);
extern kern_return_t task_swapin(
	task_t,                                 /* task */
	boolean_t);                             /* make_unswappable */
extern kern_return_t task_swapout(task_t /* task */);
extern void task_swapper(void);
extern void task_swap_swapout_thread(void);
extern void compute_vm_averages(void);
extern kern_return_t task_swappable(
	host_priv_t,                            /* host */
	task_t,                                 /* task */
	boolean_t);                             /* swappable */
extern void task_swapout_eligible(task_t /* task */);
extern void task_swapout_ineligible(task_t /* task */);
extern void swapout_ast(void);

#endif  /* _KERN_TASK_SWAP_H_ */
