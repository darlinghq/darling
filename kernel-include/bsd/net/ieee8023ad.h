/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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
 * ieee8023ad.h
 */

/*
 * Modification History
 *
 * May 14, 2004	Dieter Siegmund (dieter@apple.com)
 * - created
 */


#ifndef _NET_IEEE8023AD_H_
#define _NET_IEEE8023AD_H_

#include <sys/types.h>

#define IEEE8023AD_SLOW_PROTO_ETHERTYPE                         0x8809
#define IEEE8023AD_SLOW_PROTO_MULTICAST { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x02 }

#define IEEE8023AD_SLOW_PROTO_SUBTYPE_LACP                      1
#define IEEE8023AD_SLOW_PROTO_SUBTYPE_LA_MARKER_PROTOCOL        2
#define IEEE8023AD_SLOW_PROTO_SUBTYPE_RESERVED_START            3
#define IEEE8023AD_SLOW_PROTO_SUBTYPE_RESERVED_END              10
#endif /* _NET_IEEE8023AD_H_ */
