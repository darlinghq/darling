/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)if.h	8.1 (Berkeley) 6/10/93
 */

#ifndef _NET_IF_H_
#define _NET_IF_H_

#include <sys/cdefs.h>
#include <net/net_kev.h>

#define IF_NAMESIZE     16

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#include <sys/appleapiopts.h>
#ifdef __APPLE__

#include <net/if_var.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef PRIVATE
#include <net/if_dl.h>
#include <netinet/in.h>
#include <mach/vm_types.h>
#endif /* PRIVATE */
#endif

struct if_clonereq {
	int     ifcr_total;             /* total cloners (out) */
	int     ifcr_count;             /* room for this many in user buffer */
	char    *ifcr_buffer;           /* buffer for cloner names */
};

#ifdef KERNEL_PRIVATE
#define IF_MAXUNIT      0x7fff  /* historical value */

struct if_clonereq64 {
	int     ifcr_total;             /* total cloners (out) */
	int     ifcr_count;             /* room for this many in user buffer */
	user64_addr_t ifcru_buffer      __attribute__((aligned(8)));
};

struct if_clonereq32 {
	int     ifcr_total;             /* total cloners (out) */
	int     ifcr_count;             /* room for this many in user buffer */
	user32_addr_t ifcru_buffer;
};
#endif /* KERNEL_PRIVATE */

#define IFF_UP          0x1             /* interface is up */
#define IFF_BROADCAST   0x2             /* broadcast address valid */
#define IFF_DEBUG       0x4             /* turn on debugging */
#define IFF_LOOPBACK    0x8             /* is a loopback net */
#define IFF_POINTOPOINT 0x10            /* interface is point-to-point link */
#define IFF_NOTRAILERS  0x20            /* obsolete: avoid use of trailers */
#define IFF_RUNNING     0x40            /* resources allocated */
#define IFF_NOARP       0x80            /* no address resolution protocol */
#define IFF_PROMISC     0x100           /* receive all packets */
#define IFF_ALLMULTI    0x200           /* receive all multicast packets */
#define IFF_OACTIVE     0x400           /* transmission in progress */
#define IFF_SIMPLEX     0x800           /* can't hear own transmissions */
#define IFF_LINK0       0x1000          /* per link layer defined bit */
#define IFF_LINK1       0x2000          /* per link layer defined bit */
#define IFF_LINK2       0x4000          /* per link layer defined bit */
#define IFF_ALTPHYS     IFF_LINK2       /* use alternate physical connection */
#define IFF_MULTICAST   0x8000          /* supports multicast */

#ifdef PRIVATE
/* extended flags definitions:  (all bits reserved for internal/future use) */
#define IFEF_AUTOCONFIGURING    0x00000001      /* allow BOOTP/DHCP replies to enter */
#define IFEF_ENQUEUE_MULTI      0x00000002      /* enqueue multiple packets at once */
#define IFEF_DELAY_START        0x00000004      /* delay start callback */
#define IFEF_PROBE_CONNECTIVITY 0x00000008      /* Probe connections going over this interface */
#define IFEF_ADV_REPORT         0x00000010      /* Supports interface advisory report */
#define IFEF_IPV6_DISABLED      0x00000020      /* coupled to ND6_IFF_IFDISABLED */
#define IFEF_ACCEPT_RTADV       0x00000040      /* accepts IPv6 RA on the interface */
#define IFEF_TXSTART            0x00000080      /* has start callback */
#define IFEF_RXPOLL             0x00000100      /* supports opportunistic input poll */
#define IFEF_VLAN               0x00000200      /* interface has one or more vlans */
#define IFEF_BOND               0x00000400      /* interface is part of bond */
#define IFEF_ARPLL              0x00000800      /* ARP for IPv4LL addresses */
#define IFEF_CLAT46             0x00001000      /* CLAT46 RFC 6877 */

#define IS_INTF_CLAT46(ifp)     ((ifp) != NULL && ((ifp)->if_eflags & IFEF_CLAT46))
#define INTF_ADJUST_MTU_FOR_CLAT46(intf)                \
    (IS_INTF_CLAT46((intf)) ||                          \
     IS_INTF_CLAT46((intf)->if_delegated.ifp))          \

/*
 * XXX IFEF_NOAUTOIPV6LL is deprecated and should be done away with.
 * Configd pretty much manages the interface configuration.
 * Rather than looking at the flag we check if a specific LLA
 * has to be configured or the IID has to be generated by kernel.
 */
#define IFEF_NOAUTOIPV6LL       0x00002000      /* Need explicit IPv6 LL address */
#define IFEF_EXPENSIVE          0x00004000      /* Data access has a cost */
#define IFEF_IPV4_ROUTER        0x00008000      /* interior when in IPv4 router mode */
#define IFEF_IPV6_ROUTER        0x00010000      /* interior when in IPv6 router mode */
#define IFEF_LOCALNET_PRIVATE   0x00020000      /* local private network */
#define IFEF_SERVICE_TRIGGERED  IFEF_LOCALNET_PRIVATE
#define IFEF_IPV6_ND6ALT        0x00040000      /* alternative. KPI for ND6 */
#define IFEF_RESTRICTED_RECV    0x00080000      /* interface restricts inbound pkts */
#define IFEF_AWDL               0x00100000      /* Apple Wireless Direct Link */
#define IFEF_NOACKPRI           0x00200000      /* No TCP ACK prioritization */
#define IFEF_AWDL_RESTRICTED    0x00400000      /* Restricted AWDL mode */
#define IFEF_2KCL               0x00800000      /* prefers 2K cluster (socket based tunnel) */
#define IFEF_ECN_ENABLE         0x01000000      /* use ECN for TCP connections on the interface */
#define IFEF_ECN_DISABLE        0x02000000      /* do not use ECN for TCP connections on the interface */
#define IFEF_SKYWALK_NATIVE     0x04000000      /* Native Skywalk support */
#define IFEF_3CA                0x08000000      /* Capable of 3CA */
#define IFEF_SENDLIST           0x10000000      /* Supports tx packet lists */
#define IFEF_DIRECTLINK         0x20000000      /* point-to-point topology */
#define IFEF_QOSMARKING_ENABLED 0x40000000      /* OoS marking is enabled */
#define IFEF_UPDOWNCHANGE       0x80000000      /* up/down state is changing */

#ifdef XNU_KERNEL_PRIVATE
/*
 * Extra flags
 */
#define IFXF_WAKE_ON_MAGIC_PACKET       0x00000001 /* wake on magic packet */
#define IFXF_TIMESTAMP_ENABLED          0x00000002 /* time stamping enabled */
#define IFXF_NX_NOAUTO                  0x00000004 /* no auto config nexus */
#define IFXF_LEGACY                     0x00000008 /* legacy (non-netif) mode */
#define IFXF_LOW_INTERNET_UL            0x00000010 /* Uplink Low Internet is confirmed */
#define IFXF_LOW_INTERNET_DL            0x00000020 /* Downlink Low Internet is confirmed */
#define IFXF_ALLOC_KPI                  0x00000040 /* Allocated via the ifnet_alloc KPI */
#define IFXF_LOW_POWER                  0x00000080 /* Low Power Mode */
#define IFXF_MPK_LOG                    0x00000100 /* Multi-layer Packet Logging */
#define IFXF_CONSTRAINED                0x00000200 /* Constrained - Save Data Mode */
#define IFXF_LOW_LATENCY                0x00000400 /* Low latency interface */
/*
 * Current requirements for an AWDL interface.  Setting/clearing IFEF_AWDL
 * will also trigger the setting/clearing of the rest of the flags.  Once
 * IFEF_AWDL is set, the rest of flags cannot be cleared, by definition.
 */
#define IFEF_AWDL_MASK \
	(IFEF_LOCALNET_PRIVATE | IFEF_IPV6_ND6ALT | IFEF_RESTRICTED_RECV | \
	IFEF_AWDL)
#endif /* XNU_KERNEL_PRIVATE */
#endif /* PRIVATE */

#ifdef KERNEL_PRIVATE
/*
 * !!! NOTE !!!
 *
 * if_idle_flags definitions: (all bits are reserved for internal/future
 * use). Setting these flags MUST be done via the ifnet_set_idle_flags()
 * KPI due to the associated reference counting.  Clearing them may be done by
 * calling the KPI, otherwise implicitly at interface detach time.  Setting
 * the if_idle_flags field to a non-zero value will cause the networking
 * stack to aggressively purge expired objects (routes, etc.)
 */
#define IFRF_IDLE_NOTIFY        0x1     /* Generate notifications on idle */

/* flags set internally only: */
#define IFF_CANTCHANGE \
	(IFF_BROADCAST|IFF_POINTOPOINT|IFF_RUNNING|IFF_OACTIVE|\
	    IFF_SIMPLEX|IFF_MULTICAST|IFF_ALLMULTI)
#endif /* KERNEL_PRIVATE */

/*
 * Capabilities that interfaces can advertise.
 *
 * struct ifnet.if_capabilities
 *   contains the optional features & capabilities a particular interface
 *   supports (not only the driver but also the detected hw revision).
 *   Capabilities are defined by IFCAP_* below.
 * struct ifnet.if_capenable
 *   contains the enabled (either by default or through ifconfig) optional
 *   features & capabilities on this interface.
 *   Capabilities are defined by IFCAP_* below.
 * struct if_data.ifi_hwassist in IFNET_* form, defined in net/kpi_interface.h,
 *   contains the enabled optional features & capabilites that can be used
 *   individually per packet and are specified in the mbuf pkthdr.csum_flags
 *   field.  IFCAP_* and IFNET_* do not match one to one and IFNET_* may be
 *   more detailed or differentiated than IFCAP_*.
 *   IFNET_* hwassist flags have corresponding CSUM_* in sys/mbuf.h
 */
#define IFCAP_RXCSUM            0x00001 /* can offload checksum on RX */
#define IFCAP_TXCSUM            0x00002 /* can offload checksum on TX */
#define IFCAP_VLAN_MTU          0x00004 /* VLAN-compatible MTU */
#define IFCAP_VLAN_HWTAGGING    0x00008 /* hardware VLAN tag support */
#define IFCAP_JUMBO_MTU         0x00010 /* 9000 byte MTU supported */
#define IFCAP_TSO4              0x00020 /* can do TCP Segmentation Offload */
#define IFCAP_TSO6              0x00040 /* can do TCP6 Segmentation Offload */
#define IFCAP_LRO               0x00080 /* can do Large Receive Offload */
#define IFCAP_AV                0x00100 /* can do 802.1 AV Bridging */
#define IFCAP_TXSTATUS          0x00200 /* can return linklevel xmit status */
#define IFCAP_SKYWALK           0x00400 /* Skywalk mode supported/enabled */
#define IFCAP_HW_TIMESTAMP      0x00800 /* Time stamping in hardware */
#define IFCAP_SW_TIMESTAMP      0x01000 /* Time stamping in software */
#define IFCAP_CSUM_PARTIAL      0x02000 /* can offload partial checksum */
#define IFCAP_CSUM_ZERO_INVERT  0x04000 /* can invert 0 to -0 (0xffff) */

#define IFCAP_HWCSUM    (IFCAP_RXCSUM | IFCAP_TXCSUM)
#define IFCAP_TSO       (IFCAP_TSO4 | IFCAP_TSO6)

#define IFCAP_VALID (IFCAP_HWCSUM | IFCAP_TSO | IFCAP_LRO | IFCAP_VLAN_MTU | \
	IFCAP_VLAN_HWTAGGING | IFCAP_JUMBO_MTU | IFCAP_AV | IFCAP_TXSTATUS | \
	IFCAP_SKYWALK | IFCAP_SW_TIMESTAMP | IFCAP_HW_TIMESTAMP | \
	IFCAP_CSUM_PARTIAL | IFCAP_CSUM_ZERO_INVERT)

#define IFQ_MAXLEN      128
#define IFNET_SLOWHZ    1       /* granularity is 1 second */
#define IFQ_TARGET_DELAY        (10ULL * 1000 * 1000)   /* 10 ms */
#define IFQ_UPDATE_INTERVAL     (100ULL * 1000 * 1000)  /* 100 ms */

/*
 * Message format for use in obtaining information about interfaces
 * from sysctl and the routing socket
 */
struct if_msghdr {
	unsigned short  ifm_msglen;     /* to skip non-understood messages */
	unsigned char   ifm_version;    /* future binary compatability */
	unsigned char   ifm_type;       /* message type */
	int             ifm_addrs;      /* like rtm_addrs */
	int             ifm_flags;      /* value of if_flags */
	unsigned short  ifm_index;      /* index for associated ifp */
	struct  if_data ifm_data;       /* statistics and other data about if */
};

/*
 * Message format for use in obtaining information about interface addresses
 * from sysctl and the routing socket
 */
struct ifa_msghdr {
	unsigned short  ifam_msglen;    /* to skip non-understood messages */
	unsigned char   ifam_version;   /* future binary compatability */
	unsigned char   ifam_type;      /* message type */
	int             ifam_addrs;     /* like rtm_addrs */
	int             ifam_flags;     /* value of ifa_flags */
	unsigned short  ifam_index;     /* index for associated ifp */
	int             ifam_metric;    /* value of ifa_metric */
};

/*
 * Message format for use in obtaining information about multicast addresses
 * from the routing socket
 */
struct ifma_msghdr {
	unsigned short  ifmam_msglen;   /* to skip non-understood messages */
	unsigned char   ifmam_version;  /* future binary compatability */
	unsigned char   ifmam_type;     /* message type */
	int             ifmam_addrs;    /* like rtm_addrs */
	int             ifmam_flags;    /* value of ifa_flags */
	unsigned short  ifmam_index;    /* index for associated ifp */
};

/*
 * Message format for use in obtaining information about interfaces
 * from sysctl
 */
struct if_msghdr2 {
	u_short ifm_msglen;     /* to skip over non-understood messages */
	u_char  ifm_version;    /* future binary compatability */
	u_char  ifm_type;       /* message type */
	int     ifm_addrs;      /* like rtm_addrs */
	int     ifm_flags;      /* value of if_flags */
	u_short ifm_index;      /* index for associated ifp */
	int     ifm_snd_len;    /* instantaneous length of send queue */
	int     ifm_snd_maxlen; /* maximum length of send queue */
	int     ifm_snd_drops;  /* number of drops in send queue */
	int     ifm_timer;      /* time until if_watchdog called */
	struct if_data64        ifm_data;       /* statistics and other data */
};

/*
 * Message format for use in obtaining information about multicast addresses
 * from sysctl
 */
struct ifma_msghdr2 {
	u_short ifmam_msglen;   /* to skip over non-understood messages */
	u_char  ifmam_version;  /* future binary compatability */
	u_char  ifmam_type;     /* message type */
	int     ifmam_addrs;    /* like rtm_addrs */
	int     ifmam_flags;    /* value of ifa_flags */
	u_short ifmam_index;    /* index for associated ifp */
	int32_t ifmam_refcount;
};

/*
 * ifdevmtu: interface device mtu
 *    Used with SIOCGIFDEVMTU to get the current mtu in use by the device,
 *    as well as the minimum and maximum mtu allowed by the device.
 */
struct ifdevmtu {
	int     ifdm_current;
	int     ifdm_min;
	int     ifdm_max;
};

#pragma pack(4)

/*
 *  ifkpi: interface kpi ioctl
 *  Used with SIOCSIFKPI and SIOCGIFKPI.
 *
 *  ifk_module_id - From in the kernel, a value from kev_vendor_code_find. From
 *       user space, a value from SIOCGKEVVENDOR ioctl on a kernel event socket.
 *  ifk_type - The type. Types are specific to each module id.
 *  ifk_data - The data. ifk_ptr may be a 64bit pointer for 64 bit processes.
 *
 *  Copying data between user space and kernel space is done using copyin
 *  and copyout. A process may be running in 64bit mode. In such a case,
 *  the pointer will be a 64bit pointer, not a 32bit pointer. The following
 *  sample is a safe way to copy the data in to the kernel from either a
 *  32bit or 64bit process:
 *
 *  user_addr_t tmp_ptr;
 *  if (IS_64BIT_PROCESS(current_proc())) {
 *       tmp_ptr = CAST_USER_ADDR_T(ifkpi.ifk_data.ifk_ptr64);
 *  }
 *  else {
 *       tmp_ptr = CAST_USER_ADDR_T(ifkpi.ifk_data.ifk_ptr);
 *  }
 *  error = copyin(tmp_ptr, allocated_dst_buffer, size of allocated_dst_buffer);
 */

struct ifkpi {
	unsigned int    ifk_module_id;
	unsigned int    ifk_type;
	union {
		void            *ifk_ptr;
		int             ifk_value;
#ifdef KERNEL
		u_int64_t       ifk_ptr64;
#endif /* KERNEL */
	} ifk_data;
};

/* Wake capabilities of a interface */
#define IF_WAKE_ON_MAGIC_PACKET         0x01
#ifdef KERNEL_PRIVATE
#define IF_WAKE_VALID_FLAGS IF_WAKE_ON_MAGIC_PACKET
#endif /* KERNEL_PRIVATE */


#pragma pack()

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct  ifreq {
#ifndef IFNAMSIZ
#define IFNAMSIZ        IF_NAMESIZE
#endif
	char    ifr_name[IFNAMSIZ];             /* if name, e.g. "en0" */
	union {
		struct  sockaddr ifru_addr;
		struct  sockaddr ifru_dstaddr;
		struct  sockaddr ifru_broadaddr;
		short   ifru_flags;
		int     ifru_metric;
		int     ifru_mtu;
		int     ifru_phys;
		int     ifru_media;
		int     ifru_intval;
		caddr_t ifru_data;
#ifdef KERNEL_PRIVATE
		u_int64_t ifru_data64;  /* 64-bit ifru_data */
#endif /* KERNEL_PRIVATE */
		struct  ifdevmtu ifru_devmtu;
		struct  ifkpi   ifru_kpi;
		u_int32_t ifru_wake_flags;
		u_int32_t ifru_route_refcnt;
#ifdef PRIVATE
		int     ifru_link_quality_metric;
#endif /* PRIVATE */
		int     ifru_cap[2];
#ifdef PRIVATE
		struct {
			uint32_t        ifo_flags;
#define IFRIFOF_BLOCK_OPPORTUNISTIC     0x00000001
			uint32_t        ifo_inuse;
		} ifru_opportunistic;
		u_int64_t ifru_eflags;
		u_int64_t ifru_xflags;
		struct {
			int32_t         ifl_level;
			uint32_t        ifl_flags;
#define IFRLOGF_DLIL                    0x00000001
#define IFRLOGF_FAMILY                  0x00010000
#define IFRLOGF_DRIVER                  0x01000000
#define IFRLOGF_FIRMWARE                0x10000000
			int32_t         ifl_category;
#define IFRLOGCAT_CONNECTIVITY          1
#define IFRLOGCAT_QUALITY               2
#define IFRLOGCAT_PERFORMANCE           3
			int32_t         ifl_subcategory;
		} ifru_log;
		u_int32_t ifru_delegated;
		struct {
			uint32_t        ift_type;
			uint32_t        ift_family;
#define IFRTYPE_FAMILY_ANY              0
#define IFRTYPE_FAMILY_LOOPBACK         1
#define IFRTYPE_FAMILY_ETHERNET         2
#define IFRTYPE_FAMILY_SLIP             3
#define IFRTYPE_FAMILY_TUN              4
#define IFRTYPE_FAMILY_VLAN             5
#define IFRTYPE_FAMILY_PPP              6
#define IFRTYPE_FAMILY_PVC              7
#define IFRTYPE_FAMILY_DISC             8
#define IFRTYPE_FAMILY_MDECAP           9
#define IFRTYPE_FAMILY_GIF              10
#define IFRTYPE_FAMILY_FAITH            11
#define IFRTYPE_FAMILY_STF              12
#define IFRTYPE_FAMILY_FIREWIRE         13
#define IFRTYPE_FAMILY_BOND             14
#define IFRTYPE_FAMILY_CELLULAR         15
#define IFRTYPE_FAMILY_6LOWPAN          16
#define IFRTYPE_FAMILY_UTUN             17
#define IFRTYPE_FAMILY_IPSEC            18
			uint32_t        ift_subfamily;
#define IFRTYPE_SUBFAMILY_ANY           0
#define IFRTYPE_SUBFAMILY_USB           1
#define IFRTYPE_SUBFAMILY_BLUETOOTH     2
#define IFRTYPE_SUBFAMILY_WIFI          3
#define IFRTYPE_SUBFAMILY_THUNDERBOLT   4
#define IFRTYPE_SUBFAMILY_RESERVED      5
#define IFRTYPE_SUBFAMILY_INTCOPROC     6
#define IFRTYPE_SUBFAMILY_QUICKRELAY    7
#define IFRTYPE_SUBFAMILY_DEFAULT       8
		} ifru_type;
#endif /* PRIVATE */
		u_int32_t ifru_functional_type;
#define IFRTYPE_FUNCTIONAL_UNKNOWN              0
#define IFRTYPE_FUNCTIONAL_LOOPBACK             1
#define IFRTYPE_FUNCTIONAL_WIRED                2
#define IFRTYPE_FUNCTIONAL_WIFI_INFRA           3
#define IFRTYPE_FUNCTIONAL_WIFI_AWDL            4
#define IFRTYPE_FUNCTIONAL_CELLULAR             5
#define IFRTYPE_FUNCTIONAL_INTCOPROC            6
#define IFRTYPE_FUNCTIONAL_COMPANIONLINK        7
#define IFRTYPE_FUNCTIONAL_LAST                 7
#ifdef PRIVATE
		u_int32_t ifru_expensive;
		u_int32_t ifru_constrained;
		u_int32_t ifru_2kcl;
		struct {
			u_int32_t qlen;
			u_int32_t timeout;
		} ifru_start_delay;
		struct if_interface_state       ifru_interface_state;
		u_int32_t ifru_probe_connectivity;
		u_int32_t ifru_ecn_mode;
#define IFRTYPE_ECN_DEFAULT             0
#define IFRTYPE_ECN_ENABLE              1
#define IFRTYPE_ECN_DISABLE             2
		u_int32_t ifru_qosmarking_mode;
#define IFRTYPE_QOSMARKING_MODE_NONE            0
#define IFRTYPE_QOSMARKING_FASTLANE     1       /* supported: socket/channel */
#define IFRTYPE_QOSMARKING_RFC4594      2       /* supported: channel only */
		u_int32_t ifru_qosmarking_enabled;
		u_int32_t ifru_disable_output;
		u_int32_t ifru_low_internet;
#define IFRTYPE_LOW_INTERNET_DISABLE_UL_DL      0x0000
#define IFRTYPE_LOW_INTERNET_ENABLE_UL          0x0001
#define IFRTYPE_LOW_INTERNET_ENABLE_DL          0x0002
		int ifru_low_power_mode;
		u_int32_t ifru_tcp_kao_max;
		int ifru_mpk_log;        /* Multi Layer Packet Log */
		u_int32_t ifru_noack_prio;
#endif /* PRIVATE */
	} ifr_ifru;
#define ifr_addr        ifr_ifru.ifru_addr      /* address */
#define ifr_dstaddr     ifr_ifru.ifru_dstaddr   /* other end of p-to-p link */
#define ifr_broadaddr   ifr_ifru.ifru_broadaddr /* broadcast address */
#ifdef __APPLE__
#define ifr_flags       ifr_ifru.ifru_flags     /* flags */
#else
#define ifr_flags       ifr_ifru.ifru_flags[0]  /* flags */
#define ifr_prevflags   ifr_ifru.ifru_flags[1]  /* flags */
#endif /* __APPLE__ */
#define ifr_metric      ifr_ifru.ifru_metric    /* metric */
#define ifr_mtu         ifr_ifru.ifru_mtu       /* mtu */
#define ifr_phys        ifr_ifru.ifru_phys      /* physical wire */
#define ifr_media       ifr_ifru.ifru_media     /* physical media */
#define ifr_data        ifr_ifru.ifru_data      /* for use by interface */
#define ifr_devmtu      ifr_ifru.ifru_devmtu
#define ifr_intval      ifr_ifru.ifru_intval    /* integer value */
#ifdef KERNEL_PRIVATE
#define ifr_data64      ifr_ifru.ifru_data64    /* 64-bit pointer */
#endif /* KERNEL_PRIVATE */
#define ifr_kpi         ifr_ifru.ifru_kpi
#define ifr_wake_flags  ifr_ifru.ifru_wake_flags /* wake capabilities */
#define ifr_route_refcnt ifr_ifru.ifru_route_refcnt /* route references count */
#ifdef PRIVATE
#define ifr_link_quality_metric ifr_ifru.ifru_link_quality_metric /* LQM */
#endif /* PRIVATE */
#define ifr_reqcap      ifr_ifru.ifru_cap[0]    /* requested capabilities */
#define ifr_curcap      ifr_ifru.ifru_cap[1]    /* current capabilities */
#ifdef PRIVATE
#define ifr_opportunistic       ifr_ifru.ifru_opportunistic
#define ifr_eflags      ifr_ifru.ifru_eflags    /* extended flags  */
#define ifr_xflags      ifr_ifru.ifru_xflags    /* extra flags  */
#define ifr_log         ifr_ifru.ifru_log       /* logging level/flags */
#define ifr_delegated   ifr_ifru.ifru_delegated /* delegated interface index */
#define ifr_expensive   ifr_ifru.ifru_expensive
#define ifr_constrained   ifr_ifru.ifru_constrained
#define ifr_type        ifr_ifru.ifru_type      /* interface type */
#define ifr_functional_type     ifr_ifru.ifru_functional_type
#define ifr_2kcl        ifr_ifru.ifru_2kcl
#define ifr_start_delay_qlen    ifr_ifru.ifru_start_delay.qlen
#define ifr_start_delay_timeout ifr_ifru.ifru_start_delay.timeout
#define ifr_interface_state     ifr_ifru.ifru_interface_state
#define ifr_probe_connectivity  ifr_ifru.ifru_probe_connectivity
#define ifr_ecn_mode    ifr_ifru.ifru_ecn_mode
#define ifr_qosmarking_mode     ifr_ifru.ifru_qosmarking_mode
#define ifr_fastlane_capable    ifr_qosmarking_mode
#define ifr_qosmarking_enabled  ifr_ifru.ifru_qosmarking_enabled
#define ifr_fastlane_enabled    ifr_qosmarking_enabled
#define ifr_disable_output      ifr_ifru.ifru_disable_output
#define ifr_low_internet        ifr_ifru.ifru_low_internet
#define ifr_low_power_mode      ifr_ifru.ifru_low_power_mode
#define ifr_tcp_kao_max         ifr_ifru.ifru_tcp_kao_max
#define ifr_mpk_log             ifr_ifru.ifru_mpk_log
#define ifr_noack_prio          ifr_ifru.ifru_noack_prio

#endif /* PRIVATE */
};

#define _SIZEOF_ADDR_IFREQ(ifr) \
	((ifr).ifr_addr.sa_len > sizeof (struct sockaddr) ? \
	(sizeof (struct ifreq) - sizeof (struct sockaddr) + \
	(ifr).ifr_addr.sa_len) : sizeof (struct ifreq))

struct ifaliasreq {
	char    ifra_name[IFNAMSIZ];            /* if name, e.g. "en0" */
	struct  sockaddr ifra_addr;
	struct  sockaddr ifra_broadaddr;
	struct  sockaddr ifra_mask;
};

struct rslvmulti_req {
	struct sockaddr *sa;
	struct sockaddr **llsa;
};

#if !defined(KERNEL) || defined(KERNEL_PRIVATE)
#pragma pack(4)

struct ifmediareq {
	char    ifm_name[IFNAMSIZ];     /* if name, e.g. "en0" */
	int     ifm_current;            /* current media options */
	int     ifm_mask;               /* don't care mask */
	int     ifm_status;             /* media status */
	int     ifm_active;             /* active options */
	int     ifm_count;              /* # entries in ifm_ulist array */
	int     *ifm_ulist;             /* media words */
};

#pragma pack()
#endif /* !KERNEL || KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE
#pragma pack(4)
struct ifmediareq64 {
	char    ifm_name[IFNAMSIZ];     /* if name, e.g. "en0" */
	int     ifm_current;            /* current media options */
	int     ifm_mask;               /* don't care mask */
	int     ifm_status;             /* media status */
	int     ifm_active;             /* active options */
	int     ifm_count;              /* # entries in ifm_ulist array */
	user64_addr_t ifmu_ulist __attribute__((aligned(8)));
};

struct ifmediareq32 {
	char    ifm_name[IFNAMSIZ];     /* if name, e.g. "en0" */
	int     ifm_current;            /* current media options */
	int     ifm_mask;               /* don't care mask */
	int     ifm_status;             /* media status */
	int     ifm_active;             /* active options */
	int     ifm_count;              /* # entries in ifm_ulist array */
	user32_addr_t ifmu_ulist;       /* 32-bit pointer */
};
#pragma pack()
#endif /* KERNEL_PRIVATE */


#pragma pack(4)
struct  ifdrv {
	char            ifd_name[IFNAMSIZ];     /* if name, e.g. "en0" */
	unsigned long   ifd_cmd;
	size_t          ifd_len;                /* length of ifd_data buffer */
	void            *ifd_data;
};
#pragma pack()

#ifdef KERNEL_PRIVATE
#pragma pack(4)
struct ifdrv32 {
	char            ifd_name[IFNAMSIZ];     /* if name, e.g. "en0" */
	u_int32_t       ifd_cmd;
	u_int32_t       ifd_len;
	user32_addr_t   ifd_data;
};

struct  ifdrv64 {
	char            ifd_name[IFNAMSIZ];     /* if name, e.g. "en0" */
	u_int64_t       ifd_cmd;
	u_int64_t       ifd_len;
	user64_addr_t   ifd_data;
};
#pragma pack()
#endif /* KERNEL_PRIVATE */

/*
 * Structure used to retrieve aux status data from interfaces.
 * Kernel suppliers to this interface should respect the formatting
 * needed by ifconfig(8): each line starts with a TAB and ends with
 * a newline.
 */

#define IFSTATMAX       800             /* 10 lines of text */
struct ifstat {
	char    ifs_name[IFNAMSIZ];     /* if name, e.g. "en0" */
	char    ascii[IFSTATMAX + 1];
};

#if !defined(KERNEL) || defined(KERNEL_PRIVATE)
/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
#pragma pack(4)
struct  ifconf {
	int     ifc_len;                /* size of associated buffer */
	union {
		caddr_t ifcu_buf;
		struct  ifreq *ifcu_req;
	} ifc_ifcu;
};
#pragma pack()
#define ifc_buf ifc_ifcu.ifcu_buf       /* buffer address */
#define ifc_req ifc_ifcu.ifcu_req       /* array of structures returned */
#endif /* !KERNEL || KERNEL_PRIVATE */

#if defined(KERNEL_PRIVATE)
#pragma pack(4)
struct ifconf32 {
	int     ifc_len;                /* size of associated buffer */
	struct {
		user32_addr_t ifcu_req;
	} ifc_ifcu;
};

struct ifconf64 {
	int     ifc_len;                /* size of associated buffer */
	struct {
		user64_addr_t ifcu_req  __attribute__((aligned(8)));
	} ifc_ifcu;
};
#pragma pack()
#endif /* KERNEL_PRIVATE */

/*
 * DLIL KEV_DL_PROTO_ATTACHED/DETACHED structure
 */
struct kev_dl_proto_data {
	struct net_event_data           link_data;
	u_int32_t                       proto_family;
	u_int32_t                       proto_remaining_count;
};

#ifdef PRIVATE
/*
 * Link Quality Metrics
 *
 *	IFNET_LQM_THRESH_OFF      Metric is not available; device is off.
 *	IFNET_LQM_THRESH_UNKNOWN  Metric is not (yet) known.
 *	IFNET_LQM_THRESH_BAD	  Link quality is considered bad by driver.
 *	IFNET_LQM_THRESH_POOR     Link quality is considered poor by driver.
 *	IFNET_LQM_THRESH_GOOD     Link quality is considered good by driver.
 */
enum {
	IFNET_LQM_THRESH_OFF            = (-2),
	IFNET_LQM_THRESH_UNKNOWN        = (-1),
	IFNET_LQM_THRESH_ABORT          = 10,
	IFNET_LQM_THRESH_MINIMALLY_VIABLE = 20,
	IFNET_LQM_THRESH_POOR           = 50,
	IFNET_LQM_THRESH_GOOD           = 100
};
#define IFNET_LQM_THRESH_BAD    IFNET_LQM_THRESH_ABORT

#ifdef XNU_KERNEL_PRIVATE
#define IFNET_LQM_MIN   IFNET_LQM_THRESH_OFF
#define IFNET_LQM_MAX   IFNET_LQM_THRESH_GOOD
#endif /* XNU_KERNEL_PRIVATE */

/*
 * DLIL KEV_DL_LINK_QUALITY_METRIC_CHANGED structure
 */
struct kev_dl_link_quality_metric_data {
	struct net_event_data   link_data;
	int                     link_quality_metric;
};

#define IF_DESCSIZE     128

/*
 * Structure for SIOC[SG]IFDESC
 */
struct if_descreq {
	char                    ifdr_name[IFNAMSIZ];    /* interface name */
	u_int32_t               ifdr_len;               /* up to IF_DESCSIZE */
	u_int8_t                ifdr_desc[IF_DESCSIZE]; /* opaque data */
};

/*
 *	Output packet scheduling models
 *
 *	IFNET_SCHED_MODEL_NORMAL The default output packet scheduling model
 *		where the driver or media does not require strict scheduling
 *		strategy, and that the networking stack is free to choose the
 *		most appropriate scheduling and queueing algorithm, including
 *		shaping traffics.
 *	IFNET_SCHED_MODEL_DRIVER_MANAGED The alternative output packet
 *		scheduling model where the driver or media requires strict
 *		scheduling strategy (e.g. 802.11 WMM), and that the networking
 *		stack is only responsible for creating multiple queues for the
 *		corresponding service classes.
 */
enum {
	IFNET_SCHED_MODEL_NORMAL                = 0,
	IFNET_SCHED_MODEL_DRIVER_MANAGED        = 1,
	IFNET_SCHED_MODEL_FQ_CODEL              = 2,
#ifdef XNU_KERNEL_PRIVATE
	IFNET_SCHED_MODEL_MAX                   = 3,
#endif /* XNU_KERNEL_PRIVATE */
};

/*
 * Values for iflpr_flags
 */
#define IFLPRF_ALTQ             0x1     /* configured via PF/ALTQ */
#define IFLPRF_DRVMANAGED       0x2     /* output queue scheduled by drv */

/*
 * Structure for SIOCGIFLINKPARAMS
 */
struct if_linkparamsreq {
	char            iflpr_name[IFNAMSIZ];   /* interface name */
	u_int32_t       iflpr_flags;
	u_int32_t       iflpr_output_sched;
	u_int64_t       iflpr_output_tbr_rate;
	u_int32_t       iflpr_output_tbr_percent;
	u_int64_t       iflpr_input_tbr_rate;
	struct if_bandwidths iflpr_output_bw;
	struct if_bandwidths iflpr_input_bw;
	struct if_latencies iflpr_output_lt;
	struct if_latencies iflpr_input_lt;
	struct if_netem_params iflpr_input_netem;
	struct if_netem_params iflpr_output_netem;
};

/*
 * Structure for SIOCGIFQUEUESTATS
 */
struct if_qstatsreq {
	char            ifqr_name[IFNAMSIZ];    /* interface name */
	u_int32_t       ifqr_slot;
	void            *ifqr_buf               __attribute__((aligned(8)));
	int              ifqr_len               __attribute__((aligned(8)));
};

/*
 * Node Proximity Metrics
 */
enum {
	IFNET_NPM_THRESH_UNKNOWN        = (-1),
	IFNET_NPM_THRESH_NEAR           = 30,
	IFNET_NPM_THRESH_GENERAL        = 70,
	IFNET_NPM_THRESH_FAR            = 100,
};

/*
 *	Received Signal Strength Indication [special values]
 *
 *	IFNET_RSSI_UNKNOWN	Metric is not (yet) known.
 */
enum {
	IFNET_RSSI_UNKNOWN      = ((-2147483647) - 1),    /* INT32_MIN */
};


/*
 * DLIL KEV_DL_NODE_PRESENCE/KEV_DL_NODE_ABSENCE event structures
 */
struct kev_dl_node_presence {
	struct net_event_data   link_data;
	struct sockaddr_in6     sin6_node_address;
	struct sockaddr_dl      sdl_node_address;
	int32_t                 rssi;
	int                     link_quality_metric;
	int                     node_proximity_metric;
	u_int8_t                node_service_info[48];
};

struct kev_dl_node_absence {
	struct net_event_data   link_data;
	struct sockaddr_in6     sin6_node_address;
	struct sockaddr_dl      sdl_node_address;
};

/*
 * Structure for SIOC[SG]IFTHROTTLE
 */
struct if_throttlereq {
	char            ifthr_name[IFNAMSIZ];   /* interface name */
	u_int32_t       ifthr_level;
};

/*
 *	Interface throttling levels
 *
 *	IFNET_THROTTLE_OFF The default throttling level (no throttling.)
 *		All service class queues operate normally according to the
 *		standard packet scheduler configuration.
 *	IFNET_THROTTLE_OPPORTUNISTIC One or more service class queues that
 *		are responsible for managing "opportunistic" traffics are
 *		suspended.  Packets enqueued on those queues will be dropped
 *		and a flow advisory error will be generated to the data
 *		source.  Existing packets in the queues will stay enqueued
 *		until the interface is no longer throttled, or until they
 *		are explicitly flushed.
 */
enum {
	IFNET_THROTTLE_OFF                      = 0,
	IFNET_THROTTLE_OPPORTUNISTIC            = 1,
#ifdef XNU_KERNEL_PRIVATE
	IFNET_THROTTLE_MAX                      = 2,
#endif /* XNU_KERNEL_PRIVATE */
};

/*
 * Structure for SIOC[A/D]IFAGENTID
 */
struct if_agentidreq {
	char            ifar_name[IFNAMSIZ];    /* interface name */
	uuid_t          ifar_uuid;              /* agent UUID to add or delete */
};

/*
 * Structure for SIOCGIFAGENTIDS
 */
struct if_agentidsreq {
	char            ifar_name[IFNAMSIZ];    /* interface name */
	u_int32_t       ifar_count;             /* number of agent UUIDs */
	uuid_t          *ifar_uuids;            /* array of agent UUIDs */
};

#ifdef BSD_KERNEL_PRIVATE
struct if_agentidsreq32 {
	char            ifar_name[IFNAMSIZ];
	u_int32_t       ifar_count;
	user32_addr_t ifar_uuids;
};
struct if_agentidsreq64 {
	char            ifar_name[IFNAMSIZ];
	u_int32_t       ifar_count;
	user64_addr_t ifar_uuids __attribute__((aligned(8)));
};
#endif /* BSD_KERNEL_PRIVATE */

/*
 * Structure for SIOCGIFNEXUS
 */
struct if_nexusreq {
	char            ifnr_name[IFNAMSIZ];    /* interface name */
	uint64_t        ifnr_flags;             /* unused, must be zero */
	uuid_t          ifnr_netif;             /* netif nexus instance UUID */
	uuid_t          ifnr_flowswitch;        /* flowswitch nexus UUID */
	uint64_t        ifnr_reserved[5];
};

#define DLIL_MODIDLEN   20      /* same as IFNET_MODIDLEN */
#define DLIL_MODARGLEN  12      /* same as IFNET_MODARGLEN */

/*
 * DLIL KEV_DL_ISSUES event structure
 */
struct kev_dl_issues {
	struct net_event_data   link_data;
	u_int8_t                modid[DLIL_MODIDLEN];
	u_int64_t               timestamp;
	u_int8_t                info[DLIL_MODARGLEN];
};

/*
 * DLIL KEV_DL_RRC_STATE_CHANGED structure
 */
struct kev_dl_rrc_state {
	struct net_event_data   link_data;
	u_int32_t               rrc_state;
};

/*
 * KEV_DL_LOW_POWER_MODE_CHANGED
 */
struct kev_dl_low_power_mode {
	struct net_event_data   link_data;
	int                     low_power_event;
};

/*
 * Length of network signature/fingerprint blob.
 */
#define IFNET_SIGNATURELEN      20

/*
 * Structure for SIOC[S/G]IFNETSIGNATURE
 */
struct if_nsreq {
	char            ifnsr_name[IFNAMSIZ];
	u_int8_t        ifnsr_family;   /* address family */
	u_int8_t        ifnsr_len;      /* data length */
	u_int16_t       ifnsr_flags;    /* for future */
	u_int8_t        ifnsr_data[IFNET_SIGNATURELEN];
};


#define NAT64_PREFIX_LEN_32     4
#define NAT64_PREFIX_LEN_40     5
#define NAT64_PREFIX_LEN_48     6
#define NAT64_PREFIX_LEN_56     7
#define NAT64_PREFIX_LEN_64     8
#define NAT64_PREFIX_LEN_96     12
#define NAT64_PREFIX_LEN_MAX    NAT64_PREFIX_LEN_96

#define NAT64_MAX_NUM_PREFIXES  4

struct ipv6_prefix {
	struct in6_addr ipv6_prefix;
	uint32_t        prefix_len;
};

struct if_ipv6_address {
	struct in6_addr v6_address;
	uint32_t        v6_prefixlen;
};

/* Structure for SIOC[S/G]IFNAT64PREFIX */
struct if_nat64req {
	char                    ifnat64_name[IFNAMSIZ];
	struct ipv6_prefix      ifnat64_prefixes[NAT64_MAX_NUM_PREFIXES];
};

/* Structure for SIOCGIFCLAT46ADDR */
struct if_clat46req {
	char                    ifclat46_name[IFNAMSIZ];
	struct if_ipv6_address  ifclat46_addr;
};

/*
 * Structure for SIOC[S/G]IFORDER
 *
 * When setting, ifo_count is the number of u_int32_t interface indices
 * in the ifo_ordered_indices array.
 *
 * When getting, if ifo_count is 0, the length of the ordered list will
 * be returned. If the ifo_count is non-0, it is the number of u_int32_t
 * interface indices allocated. Upon return, ifo_count will contain the number
 * of indices copied into the array.
 */
struct if_order {
	u_int32_t                       ifo_count;
	u_int32_t                       ifo_reserved;
	mach_vm_address_t       ifo_ordered_indices; /* array of u_int32_t */
};

/*
 * Struct for traffic class to DSCP mapping
 */
struct if_tdmreq {
	char                            iftdm_name[IFNAMSIZ];
	u_int32_t                       iftdm_len;      /* byte length of the table */
	struct netsvctype_dscp_map      *iftdm_table;
};

#ifdef BSD_KERNEL_PRIVATE
struct if_tdmreq32 {
	char                    iftdm_name[IFNAMSIZ];
	u_int32_t               iftdm_len;      /* byte length of the table */
	user32_addr_t           iftdm_table;
};

struct if_tdmreq64 {
	char                    iftdm_name[IFNAMSIZ];
	u_int32_t               iftdm_len;      /* byte length of the table */
	user64_addr_t           iftdm_table __attribute__((aligned(8)));
};
#endif

/*
 * Structure for SIOCGIFPROTOLIST.
 */
struct if_protolistreq {
	char                    ifpl_name[IFNAMSIZ];
	u_int32_t               ifpl_count;
	u_int32_t               ifpl_reserved; /* must be zero */
	u_int32_t               *ifpl_list;
};

#ifdef BSD_KERNEL_PRIVATE
struct if_protolistreq32 {
	char                    ifpl_name[IFNAMSIZ];
	u_int32_t               ifpl_count;
	u_int32_t               ifpl_reserved; /* must be zero */
	user32_addr_t           ifpl_list;
};

struct if_protolistreq64 {
	char                    ifpl_name[IFNAMSIZ];
	u_int32_t               ifpl_count;
	u_int32_t               ifpl_reserved; /* must be zero */
	user64_addr_t           ifpl_list;
};
#endif /* BSD_KERNEL_PRIVATE */

#endif /* PRIVATE */

#ifdef KERNEL
#ifdef MALLOC_DECLARE
MALLOC_DECLARE(M_IFADDR);
MALLOC_DECLARE(M_IFMADDR);
#endif
#endif
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

#ifndef KERNEL
struct if_nameindex {
	unsigned int     if_index;      /* 1, 2, ... */
	char            *if_name;       /* null terminated name: "le0", ... */
};

__BEGIN_DECLS
unsigned int     if_nametoindex(const char *);
char            *if_indextoname(unsigned int, char *);
struct           if_nameindex *if_nameindex(void);
void             if_freenameindex(struct if_nameindex *);
__END_DECLS
#endif

#ifdef KERNEL
#include <net/kpi_interface.h>
#endif

#endif /* !_NET_IF_H_ */
