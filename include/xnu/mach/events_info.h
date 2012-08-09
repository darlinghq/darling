/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
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
 * @OSF_FREE_COPYRIGHT@
 */
/*
 *	Machine-independent event information structures and definitions.
 *
 *	The definitions in this file are exported to the user.  The kernel
 *	will translate its internal data structures to these structures
 *	as appropriate.
 *
 *	This data structure is used to track events that occur during
 *	thread execution, and to summarize this information for tasks.
 */

#ifndef	_MACH_EVENTS_INFO_H_
#define _MACH_EVENTS_INFO_H_

#include <mach/message.h>

struct events_info {
	integer_t	faults;		/* number of page faults */
	integer_t	zero_fills;	/* number of zero fill pages */
	integer_t	reactivations;	/* number of reactivated pages */
	integer_t	pageins;	/* number of actual pageins */
	integer_t	cow_faults;	/* number of copy-on-write faults */
	integer_t	messages_sent;	/* number of messages sent */
	integer_t	messages_received; /* number of messages received */
};
typedef struct events_info		events_info_data_t;
typedef struct events_info		*events_info_t;
#define EVENTS_INFO_COUNT	((mach_msg_type_number_t) \
		(sizeof(events_info_data_t) / sizeof(integer_t)))

#endif	/*_MACH_EVENTS_INFO_H_*/
