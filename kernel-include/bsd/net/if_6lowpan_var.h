/*
 * Copyright (c) 2017 Apple Computer, Inc. All rights reserved.
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

#ifndef _NET_IF_6LOWPAN_VAR_H_
#define _NET_IF_6LOWPAN_VAR_H_  1

#define IEEE802154_ADDR_LEN     8
#define IEEE802154_ENCAP_LEN    25 /* len of 802.15.4 Frame header */
#define IEEE802154_FRAME_LEN    127

/*
 * Configuration structure for SIOCSET6LOWPAN and SIOCGET6LOWPAN ioctls.
 */
struct sixlowpanreq {
	char parent[IFNAMSIZ];
};

#ifdef KERNEL_PRIVATE
int sixlowpan_family_init(void);
#endif /* KERNEL_PRIVATE */
#endif /* _NET_IF_6LOWPAN_VAR_H_ */
