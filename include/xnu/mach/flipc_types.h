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
 * Revision 1.1.1.1  1998/09/22 21:05:30  wsanchez
 * Import of Mac OS X kernel (~semeria)
 *
 * Revision 1.1.1.1  1998/03/07 02:25:45  wsanchez
 * Import of OSF Mach kernel (~mburg)
 *
 * Revision 1.1.7.1  1996/09/17  16:34:35  bruel
 * 	fixed types.
 * 	[96/09/17            bruel]
 *
 * Revision 1.1.5.1  1995/06/13  18:20:20  sjs
 * 	Merged from flipc_shared.
 * 	[95/06/07            sjs]
 * 
 * Revision 1.1.3.11  1995/05/23  19:55:36  randys
 * 	Don't keep track of messages sent to a bad destination--that's
 * 	purely a transport function now.
 * 	[95/05/23            randys]
 * 
 * Revision 1.1.3.10  1995/05/23  15:40:20  randys
 * 	Added field to FLIPC_domain_errors to indicate validity of other
 * 	fields.
 * 	[95/05/22            randys]
 * 
 * Revision 1.1.3.9  1995/05/16  20:46:35  randys
 * 	Added a "performance_valid" field to the flipc performance
 * 	structure.
 * 	[95/05/16            randys]
 * 
 * Revision 1.1.3.8  1995/04/05  21:22:01  randys
 * 	Added field to domain_info struct to include allocations lock
 * 	sched policy.
 * 	[95/04/05            randys]
 * 
 * Revision 1.1.3.7  1995/03/09  19:42:33  rwd
 * 	Define SEMAPHORE_NULL (for now) and include mach_types.h instead
 * 	of sema_types.h.
 * 	[95/03/08            rwd]
 * 
 * Revision 1.1.3.6  1995/02/23  21:32:52  randys
 * 	 Removed placeholder definition for locks--I don't believe that I
 * 	 use locks unless I'm on top of a real time base, in which case
 * 	 that base will define the type.
 * 	[95/02/22            randys]
 * 
 * Revision 1.1.3.5  1995/02/21  17:23:13  randys
 * 	Re-indented code to four space indentation
 * 	[1995/02/21  16:25:36  randys]
 * 
 * Revision 1.1.3.4  1995/02/16  23:20:14  randys
 * 	ANSIfy FLIPC_thread_yield_function.
 * 	[95/02/14            randys]
 * 
 * 	Add FLIPC_thread_yield_function type.
 * 	[95/02/14            randys]
 * 
 * Revision 1.1.3.3  1995/01/26  21:01:51  randys
 * 	Added performance structure.
 * 	[1995/01/24  21:14:12  randys]
 * 
 * 	Added FLIPC_epgroup_info struct
 * 	[1995/01/24  18:30:02  randys]
 * 
 * 	Create a new structure (FLIPC_endpoint_info) to return
 * 	information about an endpoint.
 * 	[1995/01/20  19:26:35  randys]
 * 
 * 	Get rid of FLIPC_DESTINATION_NULL and add in
 * 	FLIPC_ADDRESS_ERROR (return code from FLIPC_buffer_destination)
 * 	[1995/01/19  20:23:24  randys]
 * 
 * 	Added domain index type for specifying domain in
 * 	init and attach calls
 * 	[1995/01/18  16:47:25  randys]
 * 
 * Revision 1.1.3.2  1994/12/20  19:02:09  randys
 * 	Added error reporting structure type, and added
 * 	room in the standard domain query for error log size.
 * 	[1994/12/19  23:46:09  randys]
 * 
 * 	Added filename in comment at top of each file
 * 	[1994/12/19  20:28:26  randys]
 * 
 * 	Support and doc for minor user interface changes for error conditions
 * 	[1994/12/18  23:24:30  randys]
 * 
 * 	Yank the semaphore type definitions back out, and include the file
 * 	that defines those types.
 * 	[1994/12/13  17:50:03  randys]
 * 
 * Revision 1.1.3.1  1994/12/12  17:46:20  randys
 * 	Put definitions of semaphore_t and SEMAPHORE_NULL back in; they aren't
 * 	defined in user space yet.
 * 	[1994/12/12  17:21:56  randys]
 * 
 * Revision 1.1.1.2  1994/12/11  23:11:23  randys
 * 	Initial flipc code checkin
 * 
 * $EndLog$
 */

/*
 * mach/flipc_types.h
 *
 * Definitions of those flipc types that need to be visible to both the AIL
 * and kernel sides of flipc (which is just about everything).
 */

#ifndef _MACH_FLIPC_TYPES_H_
#define _MACH_FLIPC_TYPES_H_

#include <mach/port.h>

/*
 * Define a couple of generally useful types.
 */
#include <mach/mach_types.h>

#ifndef MACH_KERNEL
#define SEMAPHORE_NULL (semaphore_port_t)0
#endif /* !defined(MACH_KERNEL) */

/*
 * Basic flipc types; visible to both user and kernel segments of the
 * flipc implementation.
 */
/* Flipc addresses.  These name a node-endpoint combination for
   sending.  */
typedef unsigned int FLIPC_address_t;
#define FLIPC_ADDRESS_ERROR ((FLIPC_address_t) -1)

/* Flipc endpoints.  */
typedef void *FLIPC_endpoint_t;
#define FLIPC_ENDPOINT_NULL ((FLIPC_endpoint_t) 0)

/* Buffer pointers (returned by query functions).  Users are allowed to
   copy directly to/from this pointer; it points at their data.  */
typedef void *FLIPC_buffer_t;   
#define FLIPC_BUFFER_NULL ((FLIPC_buffer_t) 0)   

/* Endpoint group identifiers.  */
typedef void *FLIPC_epgroup_t;
#define FLIPC_EPGROUP_NULL ((FLIPC_epgroup_t) 0)
#define FLIPC_EPGROUP_ERROR ((FLIPC_epgroup_t) -1)

/* Domain index; argument to initialization and attach routines.  */
typedef unsigned int FLIPC_domain_index_t;

/* Domain handle (mach port).  */
typedef mach_port_t FLIPC_domain_t;

/* The different types an endpoint can be.  FLIPC_Inactive is used when
   the endpoint has not been configured and hence is on the freelist.  */
typedef enum {
    FLIPC_Inactive = -1,
    FLIPC_Send,
    FLIPC_Receive
} FLIPC_endpoint_type_t;

/* Structure for returning performance information about the flipc
   domain; a placeholder for future entries as needed.
   This information will only be valid if the kernel is configured to
   keep flipc performance information.  */
typedef struct FLIPC_domain_performance_info {
    unsigned long performance_valid;	/* Non zero if the other information
				   in this structure is valid.  */
    unsigned long messages_sent;		/* Since last init.  */
    unsigned long messages_received;	/* Since last init.  Includes overruns
				   (because they are marked in the
				   endpoint data structure).  Doesn't
				   include other drops (they are
				   marked in other places) */
} *FLIPC_domain_performance_info_t;

/* Flipc yield function.  */
typedef void (*FLIPC_thread_yield_function)(void);

/* Structure for returning information about the flipc domain.  */
typedef struct FLIPC_domain_info {
    int max_endpoints;
    int max_epgroups;
    int max_buffers;
    int max_buffers_per_endpoint;
    int msg_buffer_size;
    FLIPC_thread_yield_function yield_fn;
    int policy;			/* Allocations lock sched policy.
				   Unused if REAL_TIME_PRIMITIVES are
				   not being used.  */
    struct FLIPC_domain_performance_info performance;
    int error_log_size;		/* In bytes.  */
} *FLIPC_domain_info_t;

/* Structure for returning information about the error state of
   the flipc domain.  Note that this is variable sized; the size
   of the transport specific information is not known at compile
   time.  */
typedef struct FLIPC_domain_errors {
    int error_full_config_p;		/* 1 if disabled and badtype below are
					   valid; 0 if only msgdrop_inactive
					   is valid.  */ 
    int msgdrop_inactive;		/* Messages dropped because
					   of the domain being locally
					   inactive.  */
    int msgdrop_disabled;		/* Messages dropped because of a
					   disabled endpoint.  */
    int msgdrop_badtype;		/* Messages dropped because they
					   were sent to a send endpoint.  */

    int transport_error_size;	/* Size of the following array of
				   ints, in bytes.  */
    int transport_error_info[1];	/* Really of transport_error_size.  */
} *FLIPC_domain_errors_t;

/* Structure for returning information about endpoints.  */
typedef struct FLIPC_endpoint_info {
    FLIPC_endpoint_type_t type;
    unsigned int processed_buffers_dropped_p;
    unsigned long number_buffers;
    FLIPC_epgroup_t epgroup;
} *FLIPC_endpoint_info_t;

typedef struct FLIPC_epgroup_info {
    unsigned long msgs_per_wakeup;
} *FLIPC_epgroup_info_t;

#endif /* _MACH_FLIPC_TYPES_H_ */
