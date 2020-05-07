/*
 * Copyright (c) 2003,2008,2017 Apple Computer, Inc. All rights reserved.
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
/*!
 *       @header kpi_interfacefilter.h
 *       This header defines an API to attach interface filters. Interface
 *       filters may be attached to a specific interface. The filters can
 *       intercept all packets in to and out of the specific interface. In
 *       addition, the filters may intercept interface specific events and
 *       ioctls.
 */

#ifndef __KPI_INTERFACEFILTER__
#define __KPI_INTERFACEFILTER__
#include <sys/kernel_types.h>
#include <net/kpi_interface.h>

struct kev_msg;

__BEGIN_DECLS

/*!
 *       @typedef iff_input_func
 *
 *       @discussion iff_input_func is used to filter incoming packets. The
 *               interface is only valid for the duration of the filter call. If
 *               you need to keep a reference to the interface, be sure to call
 *               ifnet_reference and ifnet_release. The packets passed to the
 *               inbound filter are different from those passed to the outbound
 *               filter. Packets to the inbound filter have the frame header
 *               passed in separately from the rest of the packet. The outbound
 *               data filters is passed the whole packet including the frame
 *               header.
 *
 *               The frame header usually preceeds the data in the mbuf. This
 *               ensures that the frame header will be a valid pointer as long as
 *               the mbuf is not freed. If you need to change the frame header to
 *               point somewhere else, the recommended method is to prepend a new
 *               frame header to the mbuf chain (mbuf_prepend), set the header to
 *               point to that data, then call mbuf_adj to move the mbuf data
 *               pointer back to the start of the packet payload.
 *       @param cookie The cookie specified when this filter was attached.
 *       @param interface The interface the packet was recieved on.
 *       @param protocol The protocol of this packet. If you specified a
 *               protocol when attaching your filter, the protocol will only ever
 *               be the protocol you specified.
 *       @param data The inbound packet, after the frame header as determined
 *               by the interface.
 *       @param frame_ptr A pointer to the pointer to the frame header. The
 *               frame header length can be found by inspecting the interface's
 *               frame header length (ifnet_hdrlen).
 *       @result Return:
 *               0 - The caller will continue with normal processing of the
 *                       packet.
 *               EJUSTRETURN - The caller will stop processing the packet,
 *                       the packet will not be freed.
 *               Anything Else - The caller will free the packet and stop
 *                       processing.
 */
typedef errno_t (*iff_input_func)(void *cookie, ifnet_t interface,
    protocol_family_t protocol, mbuf_t *data, char **frame_ptr);

/*!
 *       @typedef iff_output_func
 *
 *       @discussion iff_output_func is used to filter fully formed outbound
 *               packets. The interface is only valid for the duration of the
 *               filter call. If you need to keep a reference to the interface,
 *               be sure to call ifnet_reference and ifnet_release.
 *       @param cookie The cookie specified when this filter was attached.
 *       @param interface The interface the packet is being transmitted on.
 *       @param data The fully formed outbound packet in a chain of mbufs.
 *               The frame header is already included. The filter function may
 *               modify the packet or return a different mbuf chain.
 *       @result Return:
 *               0 - The caller will continue with normal processing of the
 *                       packet.
 *               EJUSTRETURN - The caller will stop processing the packet,
 *                       the packet will not be freed.
 *               Anything Else - The caller will free the packet and stop
 *                       processing.
 */
typedef errno_t (*iff_output_func)(void *cookie, ifnet_t interface,
    protocol_family_t protocol, mbuf_t *data);

/*!
 *       @typedef iff_event_func
 *
 *       @discussion iff_event_func is used to filter interface specific
 *               events. The interface is only valid for the duration of the
 *               filter call. If you need to keep a reference to the interface,
 *               be sure to call ifnet_reference and ifnet_release.
 *       @param cookie The cookie specified when this filter was attached.
 *       @param interface The interface the packet is being transmitted on.
 *       @param event_msg The kernel event, may not be changed.
 */
typedef void (*iff_event_func)(void *cookie, ifnet_t interface,
    protocol_family_t protocol, const struct kev_msg *event_msg);

/*!
 *       @typedef iff_ioctl_func
 *
 *       @discussion iff_ioctl_func is used to filter ioctls sent to an
 *               interface. The interface is only valid for the duration of the
 *               filter call. If you need to keep a reference to the interface,
 *               be sure to call ifnet_reference and ifnet_release.
 *
 *               All undefined ioctls are reserved for future use by Apple. If
 *               you need to communicate with your kext using an ioctl, please
 *               use SIOCSIFKPI and SIOCGIFKPI.
 *       @param cookie The cookie specified when this filter was attached.
 *       @param interface The interface the packet is being transmitted on.
 *       @param ioctl_cmd The ioctl command.
 *       @param ioctl_arg A pointer to the ioctl argument.
 *       @result Return:
 *               0 - This filter function handled the ioctl.
 *               EOPNOTSUPP - This filter function does not understand/did not
 *                       handle this ioctl.
 *               EJUSTRETURN - This filter function handled the ioctl,
 *                       processing should stop.
 *               Anything Else - Processing will stop, the error will be
 *                       returned.
 */
typedef errno_t (*iff_ioctl_func)(void *cookie, ifnet_t interface,
    protocol_family_t protocol, unsigned long ioctl_cmd, void *ioctl_arg);

/*!
 *       @typedef iff_detached_func
 *
 *       @discussion iff_detached_func is called to notify the filter that it
 *               has been detached from an interface. This is the last call to
 *               the filter that will be made. A filter may be detached if the
 *               interface is detached or the detach filter function is called.
 *               In the case that the interface is being detached, your filter's
 *               event function will be called with the interface detaching event
 *               before the your detached function will be called.
 *       @param cookie The cookie specified when this filter was attached.
 *       @param interface The interface this filter was detached from.
 */
typedef void (*iff_detached_func)(void *cookie, ifnet_t interface);

/*!
 *       @struct iff_filter
 *       @discussion This structure is used to define an interface filter for
 *               use with the iflt_attach function.
 *       @field iff_cookie A kext defined cookie that will be passed to all
 *               filter functions.
 *       @field iff_name A filter name used for debugging purposes.
 *       @field iff_protocol The protocol of the packets this filter is
 *               interested in. If you specify zero, packets from all protocols
 *               will be passed to the filter.
 *       @field iff_input The filter function to handle inbound packets, may
 *               be NULL.
 *       @field iff_output The filter function to handle outbound packets,
 *               may be NULL.
 *       @field iff_event The filter function to handle interface events, may
 *               be null.
 *       @field iff_ioctl The filter function to handle interface ioctls, may
 *               be null.
 *       @field iff_detached The filter function used to notify the filter that
 *               it has been detached.
 */

struct iff_filter {
	void                    *iff_cookie;
	const char              *iff_name;
	protocol_family_t       iff_protocol;
	iff_input_func          iff_input;
	iff_output_func         iff_output;
	iff_event_func          iff_event;
	iff_ioctl_func          iff_ioctl;
	iff_detached_func       iff_detached;
};

/*!
 *       @function iflt_attach
 *       @discussion Attaches an interface filter to an interface.
 *       @param interface The interface the filter should be attached to.
 *       @param filter A structure defining the filter.
 *       @param filter_ref A reference to the filter used to detach.
 *       @result 0 on success otherwise the errno error.
 */
#ifdef KERNEL_PRIVATE
extern errno_t iflt_attach_internal(ifnet_t interface, const struct iff_filter *filter,
    interface_filter_t *filter_ref);

#define iflt_attach(interface, filter, filter_ref) \
	iflt_attach_internal((interface), (filter), (filter_ref))
#else
extern errno_t iflt_attach(ifnet_t interface, const struct iff_filter *filter,
    interface_filter_t *filter_ref);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function iflt_detach
 *       @discussion Detaches an interface filter from an interface.
 *       @param filter_ref The reference to the filter from iflt_attach.
 */
extern void iflt_detach(interface_filter_t filter_ref);

__END_DECLS
#endif /* __KPI_INTERFACEFILTER__ */
