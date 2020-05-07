/*
 * Copyright (c) 2004-2012 Apple Inc. All rights reserved.
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

#ifndef _NET_IF_ETHER_H
#define _NET_IF_ETHER_H
#ifdef KERNEL

#include <net/kpi_interface.h>

__BEGIN_DECLS
/* Not exported */
extern int ether_family_init(void);

/*
 * These functions may be used for an interface emulating an ethernet
 * interface and not using IOKit. If you use IOKit and the IOKit
 * Ethernet Family, these functions will be set for you. Use these
 * functions when filling out the ifnet_init_params structure.
 */
errno_t ether_demux(ifnet_t interface, mbuf_t packet, char* header,
    protocol_family_t *protocol);
errno_t ether_add_proto(ifnet_t interface, protocol_family_t protocol,
    const struct ifnet_demux_desc *demux_list, u_int32_t demux_count);
errno_t ether_del_proto(ifnet_t interface, protocol_family_t protocol);
#if KPI_INTERFACE_EMBEDDED
errno_t ether_frameout(ifnet_t interface, mbuf_t *packet,
    const struct sockaddr *dest, const char *dest_lladdr,
    const char *frame_type, u_int32_t *prepend_len, u_int32_t *postpend_len);
#else /* !KPI_INTERFACE_EMBEDDED */
errno_t ether_frameout(ifnet_t interface, mbuf_t *packet,
    const struct sockaddr *dest, const char *dest_lladdr,
    const char *frame_type);
#endif /* !KPI_INTERFACE_EMBEDDED */
#ifdef KERNEL_PRIVATE
errno_t ether_frameout_extended(ifnet_t interface, mbuf_t *packet,
    const struct sockaddr *dest, const char *dest_lladdr,
    const char *frame_type, u_int32_t *prepend_len, u_int32_t *postpend_len);
#endif /* KERNEL_PRIVATE */
errno_t ether_ioctl(ifnet_t interface, u_int32_t command, void* data);
errno_t ether_check_multi(ifnet_t ifp, const struct sockaddr *multicast);

__END_DECLS

#endif /* KERNEL */
#endif /* _NET_IF_ETHER_H */
