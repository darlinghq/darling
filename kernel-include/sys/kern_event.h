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
/* Copyright (c) 1998, 1999 Apple Computer, Inc. All Rights Reserved */
/*!
	@header kern_event.h
	This header defines in-kernel functions for generating kernel events as well
	as functions for receiving kernel events using a kernel event socket.
 */

#ifndef SYS_KERN_EVENT_H
#define SYS_KERN_EVENT_H

#include <sys/appleapiopts.h>
#include <sys/ioccom.h>
#include <sys/sys_domain.h>

#define KEVENTS_ON  1
#define KEV_SNDSPACE (4 * 1024)
#define KEV_RECVSPACE (32 * 1024)

#define KEV_ANY_VENDOR    0
#define KEV_ANY_CLASS     0
#define KEV_ANY_SUBCLASS  0

/*
 * Vendor Code
 */

/*!
	@defined KEV_VENDOR_APPLE
    @discussion Apple generated kernel events use the hard coded vendor code
    	value of 1. Third party kernel events use a dynamically allocated vendor
    	code. The vendor code can be found using the SIOCGKEVVENDOR ioctl.
*/
#define KEV_VENDOR_APPLE	1


/*
 * Definition of top-level classifications for KEV_VENDOR_APPLE
 */

/*!
	@defined KEV_NETWORK_CLASS
    @discussion Network kernel event class.
*/
#define KEV_NETWORK_CLASS 	1

/*!
	@defined KEV_IOKIT_CLASS
    @discussion IOKit kernel event class.
*/
#define KEV_IOKIT_CLASS   	2

/*!
	@defined KEV_IOKIT_CLASS
    @discussion System kernel event class.
*/
#define KEV_SYSTEM_CLASS  	3

/*!
	@defined KEV_APPLESHARE_CLASS
    @discussion AppleShare kernel event class.
*/
#define KEV_APPLESHARE_CLASS	4

/*!
	@defined KEV_FIREWALL_CLASS
	@discussion Firewall kernel event class.
*/
#define KEV_FIREWALL_CLASS	5

/*!
	@defined KEV_IEEE80211_CLASS
	@discussion IEEE 802.11 kernel event class.
*/
#define KEV_IEEE80211_CLASS	6

/*!
	@struct kern_event_msg
	@discussion This structure is prepended to all kernel events. This structure
		is used to determine the format of the remainder of the kernel event.
		This structure will appear on all messages received on a kernel event
		socket. To post a kernel event, a slightly different structure is used.
	@field total_size Total size of the kernel event message including the
		header.
	@field vendor_code The vendor code indicates which vendor generated the
		kernel event. This gives every vendor a unique set of classes and
		subclasses to use. Use the SIOCGKEVVENDOR ioctl to look up vendor codes
		for vendors other than Apple. Apple uses KEV_VENDOR_APPLE.
	@field kev_class The class of the kernel event.
	@field kev_subclass The subclass of the kernel event.
	@field id Monotonically increasing value.
	@field event_code The event code.
	@field event_data Any additional data about this event. Format will depend
		on the vendor_code, kev_class, kev_subclass, and event_code. The length
		of the event_data can be determined using total_size -
		KEV_MSG_HEADER_SIZE.
*/
struct kern_event_msg {
	u_int32_t	total_size;	/* Size of entire event msg */
	u_int32_t	vendor_code;	/* For non-Apple extensibility */
	u_int32_t	kev_class;	/* Layer of event source */
	u_int32_t	kev_subclass;	/* Component within layer */
	u_int32_t	id;		/* Monotonically increasing value */
	u_int32_t	event_code;	/* unique code */
	u_int32_t	event_data[1];	/* One or more data words */
};

/*!
	@defined KEV_MSG_HEADER_SIZE
	@discussion Size of the header portion of the kern_event_msg structure. This
		accounts for everything right up to event_data. The size of the data can
		be found by subtracting KEV_MSG_HEADER_SIZE from the total size from the
		kern_event_msg.
*/
#define KEV_MSG_HEADER_SIZE (offsetof(struct kern_event_msg, event_data[0]))

/*!
	@struct kev_request
	@discussion This structure is used with the SIOCSKEVFILT and SIOCGKEVFILT to
		set and get the control filter setting for a kernel control socket.
	@field total_size Total size of the kernel event message including the
		header.
	@field vendor_code All kernel events that don't match this vendor code will
		be ignored. KEV_ANY_VENDOR can be used to receive kernel events with any
		vendor code.
	@field kev_class All kernel events that don't match this class will be
		ignored. KEV_ANY_CLASS can be used to receive kernel events with any
		class.
	@field kev_subclass All kernel events that don't match this subclass will be
		ignored. KEV_ANY_SUBCLASS can be used to receive kernel events with any
		subclass.
*/
struct kev_request {
	u_int32_t	vendor_code;
	u_int32_t	kev_class;
	u_int32_t	kev_subclass;
};

/*!
	@defined KEV_VENDOR_CODE_MAX_STR_LEN
	@discussion This define sets the maximum length of a string that can be used
		to identify a vendor or kext when looking up a vendor code.
*/
#define KEV_VENDOR_CODE_MAX_STR_LEN	200

#pragma pack(4)

/*!
	@struct kev_vendor_code
	@discussion This structure is used with the SIOCGKEVVENDOR ioctl to convert
		from a string identifying a kext or vendor, in the form of a bundle
		identifier, to a vendor code.
	@field vendor_code After making the SIOCGKEVVENDOR ioctl call, this will
		be filled in with the vendor code if there is one.
	@field vendor_string A bundle style identifier.
*/
struct kev_vendor_code {
	u_int32_t	vendor_code;
	char		vendor_string[KEV_VENDOR_CODE_MAX_STR_LEN];
};

#pragma pack()

/*!
	@defined SIOCGKEVID
	@discussion Retrieve the current event id. Each event generated will have
		a new idea. The next event to be generated will have an id of id+1.
*/
#define SIOCGKEVID	_IOR('e', 1, u_int32_t)

/*!
	@defined SIOCSKEVFILT
	@discussion Set the kernel event filter for this socket. Kernel events not
		matching this filter will not be received on this socket.
*/
#define SIOCSKEVFILT	_IOW('e', 2, struct kev_request)

/*!
	@defined SIOCGKEVFILT
	@discussion Retrieve the kernel event filter for this socket. Kernel events
		not matching this filter will not be received on this socket.
*/
#define SIOCGKEVFILT    _IOR('e', 3, struct kev_request)

/*!
	@defined SIOCGKEVVENDOR
	@discussion Lookup the vendor code for the specified vendor. ENOENT will be
		returned if a vendor code for that vendor string does not exist.
*/
#define	SIOCGKEVVENDOR	_IOWR('e', 4, struct kev_vendor_code)

#ifdef KERNEL
/*!
	@define N_KEV_VECTORS
	@discussion The maximum number of kev_d_vectors for a kernel event.
*/
#define N_KEV_VECTORS     5

/*!
	@struct kev_d_vectors
	@discussion This structure is used to append some data to a kernel event.
	@field data_length The length of data.
	@field data_ptr A pointer to data.
*/
struct kev_d_vectors {
	u_int32_t	data_length;	/* Length of the event data */
	void		*data_ptr;	/* Pointer to event data */
};

/*!
	@struct kev_msg
	@discussion This structure is used when posting a kernel event.
	@field vendor_code The vendor code assigned by kev_vendor_code_find.
	@field kev_class The event's class.
	@field kev_class The event's subclass.
	@field kev_class The event's code.
	@field dv An array of vectors describing additional data to be appended to
		the kernel event.
*/
struct kev_msg {
	u_int32_t		vendor_code;		/* For non-Apple extensibility	*/
	u_int32_t		kev_class;		/* Layer of event source	*/
	u_int32_t		kev_subclass;		/* Component within layer	*/
	u_int32_t		event_code;		/* The event code		*/
	struct kev_d_vectors	dv[N_KEV_VECTORS];	/* Up to n data vectors		*/
};

/*!
	@function kev_vendor_code_find
	@discussion Lookup a vendor_code given a unique string. If the vendor code
		has not been used since launch, a unique integer will be assigned for
		that string. Vendor codes will remain the same until the machine is
		rebooted.
	@param vendor_string A bundle style vendor identifier (i.e. com.apple).
	@param vendor_code Upon return, a unique vendor code for use when posting
		kernel events.
	@result May return ENOMEM if memory constraints prevent allocation of a new
		vendor code.
 */
errno_t	kev_vendor_code_find(const char *vendor_string, u_int32_t *vendor_code);

/*!
	@function kev_msg_post
	@discussion Post a kernel event message.
	@param event_msg A structure defining the kernel event message to post.
	@result Will return zero upon success. May return a number of errors
		depending on the type of failure. EINVAL indicates that there was
		something wrong with the kerne event. The vendor code of the kernel
		event must be assigned using kev_vendor_code_find. If the message is
		too large, EMSGSIZE will be returned.
 */
errno_t kev_msg_post(struct kev_msg *event_msg);

#ifdef PRIVATE
/*
 * Internal version of kev_post_msg. Allows posting Apple vendor code kernel
 * events.
 */
int	kev_post_msg(struct kev_msg *event);

LIST_HEAD(kern_event_head, kern_event_pcb);

struct  kern_event_pcb {
	LIST_ENTRY(kern_event_pcb) ev_link;	/* glue on list of all PCBs */
	struct socket	*ev_socket;		/* pointer back to socket */
	u_int32_t	vendor_code_filter;
	u_int32_t	class_filter;
	u_int32_t	subclass_filter;
};

#define sotoevpcb(so)   ((struct kern_event_pcb *)((so)->so_pcb))


#endif /* PRIVATE */
#endif /* KERNEL */
#endif /* SYS_KERN_EVENT_H */
