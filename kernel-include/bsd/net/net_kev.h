/*
 * Copyright (c) 2016-2018 Apple Inc. All rights reserved.
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

#ifndef _NET_NETKEV_H_
#define _NET_NETKEV_H_

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)

/* Kernel event subclass identifiers for KEV_NETWORK_CLASS */
#define KEV_INET_SUBCLASS       1       /* inet subclass */
/* KEV_INET_SUBCLASS event codes */
#define KEV_INET_NEW_ADDR               1 /* Userland configured IP address */
#define KEV_INET_CHANGED_ADDR           2 /* Address changed event */
#define KEV_INET_ADDR_DELETED           3 /* IPv6 address was deleted */
#define KEV_INET_SIFDSTADDR             4 /* Dest. address was set */
#define KEV_INET_SIFBRDADDR             5 /* Broadcast address was set */
#define KEV_INET_SIFNETMASK             6 /* Netmask was set */
#define KEV_INET_ARPCOLLISION           7 /* ARP collision detected */
#ifdef __APPLE_API_PRIVATE
#define KEV_INET_PORTINUSE              8 /* use ken_in_portinuse */
#endif
#define KEV_INET_ARPRTRFAILURE          9 /* ARP resolution failed for router */
#define KEV_INET_ARPRTRALIVE            10 /* ARP resolution succeeded for router */

#define KEV_DL_SUBCLASS 2               /* Data Link subclass */
/*
 * Define Data-Link event subclass, and associated
 * events.
 */
#define KEV_DL_SIFFLAGS                         1
#define KEV_DL_SIFMETRICS                       2
#define KEV_DL_SIFMTU                           3
#define KEV_DL_SIFPHYS                          4
#define KEV_DL_SIFMEDIA                         5
#define KEV_DL_SIFGENERIC                       6
#define KEV_DL_ADDMULTI                         7
#define KEV_DL_DELMULTI                         8
#define KEV_DL_IF_ATTACHED                      9
#define KEV_DL_IF_DETACHING                     10
#define KEV_DL_IF_DETACHED                      11
#define KEV_DL_LINK_OFF                         12
#define KEV_DL_LINK_ON                          13
#define KEV_DL_PROTO_ATTACHED                   14
#define KEV_DL_PROTO_DETACHED                   15
#define KEV_DL_LINK_ADDRESS_CHANGED             16
#define KEV_DL_WAKEFLAGS_CHANGED                17
#define KEV_DL_IF_IDLE_ROUTE_REFCNT             18
#define KEV_DL_IFCAP_CHANGED                    19
#define KEV_DL_LINK_QUALITY_METRIC_CHANGED      20
#define KEV_DL_NODE_PRESENCE                    21
#define KEV_DL_NODE_ABSENCE                     22
#define KEV_DL_MASTER_ELECTED                   23
#define KEV_DL_ISSUES                           24
#define KEV_DL_IFDELEGATE_CHANGED               25
#define KEV_DL_AWDL_RESTRICTED                  26
#define KEV_DL_AWDL_UNRESTRICTED                27
#define KEV_DL_RRC_STATE_CHANGED                28
#define KEV_DL_QOS_MODE_CHANGED                 29
#define KEV_DL_LOW_POWER_MODE_CHANGED           30

#ifdef PRIVATE
#define KEV_NETPOLICY_SUBCLASS  3       /* Network policy subclass */
/* KEV_NETPOLICY_SUBCLASS event codes */
#define KEV_NETPOLICY_IFDENIED  1       /* denied access to interface */
#define KEV_NETPOLICY_IFFAILED  2       /* failed to bring up interface */

#define KEV_SOCKET_SUBCLASS     4       /* Socket subclass */
/* KEV_SOCKET_SUBCLASS event codes */
#define KEV_SOCKET_CLOSED       1       /* completely closed by protocol */
#endif /* PRIVATE */

#define KEV_INET6_SUBCLASS      6       /* inet6 subclass */
/* KEV_INET6_SUBCLASS event codes */
#define KEV_INET6_NEW_USER_ADDR         1 /* Userland configured IPv6 address */
#define KEV_INET6_CHANGED_ADDR          2 /* Address changed event (future) */
#define KEV_INET6_ADDR_DELETED          3 /* IPv6 address was deleted */
#define KEV_INET6_NEW_LL_ADDR           4 /* Autoconf LL address appeared */
#define KEV_INET6_NEW_RTADV_ADDR        5 /* Autoconf address has appeared */
#define KEV_INET6_DEFROUTER             6 /* Default router detected */
#define KEV_INET6_REQUEST_NAT64_PREFIX  7 /* Asking for the NAT64-prefix */

#ifdef PRIVATE
#define KEV_ND6_SUBCLASS        7       /* IPv6 NDP subclass */
/* KEV_ND6_SUBCLASS event codes */
#define KEV_ND6_RA                      1
#define KEV_ND6_NDFAILURE               2 /* IPv6 neighbor cache entry expiry */
#define KEV_ND6_NDALIVE                 3 /* IPv6 neighbor reachable */
#define KEV_ND6_DAD_FAILURE             4 /* IPv6 address failed DAD */
#define KEV_ND6_DAD_SUCCESS             5 /* IPv6 address completed DAD */
#define KEV_ND6_ADDR_DETACHED           6 /* IPv6 address is deemed detached */
#define KEV_ND6_ADDR_DEPRECATED         7 /* IPv6 address's preferred lifetime expired */
#define KEV_ND6_ADDR_EXPIRED            8 /* IPv6 address has expired */
#define KEV_ND6_RTR_EXPIRED             9 /* IPv6 default router has expired */
#define KEV_ND6_PFX_EXPIRED             10 /* IPv6 prefix has expired */

#define KEV_NECP_SUBCLASS       8       /* NECP subclasss */
/* KEV_NECP_SUBCLASS event codes */
#define KEV_NECP_POLICIES_CHANGED 1

#define KEV_NETAGENT_SUBCLASS   9       /* Net-Agent subclass */
/* Network Agent kernel event codes */
#define KEV_NETAGENT_REGISTERED                 1
#define KEV_NETAGENT_UNREGISTERED               2
#define KEV_NETAGENT_UPDATED                    3
#define KEV_NETAGENT_UPDATED_INTERFACES         4

#define KEV_LOG_SUBCLASS        10      /* Log subclass */
/* KEV_LOG_SUBCLASS event codes */
#define IPFWLOGEVENT    0

#define KEV_NETEVENT_SUBCLASS   11      /* Generic Net events subclass */
/* KEV_NETEVENT_SUBCLASS event codes */
#define KEV_NETEVENT_APNFALLBACK                1
#define KEV_NETEVENT_CLAT46_EVENT               2

#define KEV_MPTCP_SUBCLASS      12      /* Global MPTCP events subclass */
/* KEV_MPTCP_SUBCLASS event codes */
#define KEV_MPTCP_CELLUSE       1

#define KEV_IPSEC_SUBCLASS      13      /* IPsec event subclass */
#define KEV_IPSEC_WAKE_PACKET   1       /* IPsec wake packet available, the
	                                 * first packet processed after a wake event */

#endif /* PRIVATE */
#endif /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
#endif /* _NET_NETKEV_H_ */
