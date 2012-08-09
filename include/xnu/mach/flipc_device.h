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
 * 
 */
/*
 * HISTORY
 * 
 * Revision 1.1.1.1  1998/09/22 21:05:29  wsanchez
 * Import of Mac OS X kernel (~semeria)
 *
 * Revision 1.1.1.1  1998/03/07 02:25:45  wsanchez
 * Import of OSF Mach kernel (~mburg)
 *
 * Revision 1.1.5.1  1995/06/13  18:20:16  sjs
 * 	Merged from flipc_shared.
 * 	[95/06/07            sjs]
 *
 * Revision 1.1.3.4  1995/04/05  21:21:58  randys
 * 	Added allocations_lock_policy argument to usermsg_Init_Buffer set
 * 	status call.
 * 	[95/04/05            randys]
 * 
 * Revision 1.1.3.3  1995/02/21  17:23:08  randys
 * 	Re-indented code to four space indentation
 * 	[1995/02/21  16:25:35  randys]
 * 
 * Revision 1.1.3.2  1994/12/20  19:02:03  randys
 * 	Added filename in comment at top of each file
 * 	[1994/12/19  20:28:25  randys]
 * 
 * Revision 1.1.3.1  1994/12/12  17:46:17  randys
 * 	Putting initial flipc implementation under flipc_shared
 * 	[1994/12/12  16:27:48  randys]
 * 
 * Revision 1.1.1.2  1994/12/11  23:11:21  randys
 * 	Initial flipc code checkin
 * 
 * $EndLog$
 */

/*
 * mach/flipc_device.h
 *
 * Declarations related to the device driver interface to FLIPC.
 */

#ifndef _MACH_FLIPC_DEVICE_H_
#define _MACH_FLIPC_DEVICE_H_

/*
 * Definitions of constants both the ME and AIL need to know for
 * communications through the device driver interface.  These are the
 * possible values for the top 16 bits of the flavor parameter; the
 * bottom 16 bits are extra information that may be needed (eg. to
 * parameterize a request for semaphore in the get status routine).
 */
typedef enum {			/* Arguments.  */
    /* Get status flavors.  */	
    usermsg_Get_Initialized_Status = 1, /* (int *init_p) */
    usermsg_Get_Epgroup_Semaphore, /* (mach_port_t *semaphore) */
    usermsg_Return_Allocations_Lock, /* (void) */

    /* Set status flavors.  */
    usermsg_Init_Buffer,		/* (int max_endpoints,
					   int max_epgroups,
					   int max_buffers,
					   int max_buffers_per_endpoint,
					   int allocations_lock_policy) */
    usermsg_Process_Work,		/* (void) */
    usermsg_Acquire_Allocations_Lock, /* (void) */
    usermsg_Release_Allocations_Lock, /* (void) */
    usermsg_Epgroup_Associate_Semaphore /* (int epgroup_idx, mach_port_t port) */
} usermsg_devop_t;

#define FLIPC_DEVICE_FLAVOR(devop, param)  (((devop)<<16)|(param))

#endif /* _MACH_FLIPC_DEVICE_H_ */
