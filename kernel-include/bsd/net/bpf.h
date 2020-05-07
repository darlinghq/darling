/*
 * Copyright (c) 2000-2018 Apple Inc. All rights reserved.
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
 * Copyright (c) 1990, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from the Stanford/CMU enet packet filter,
 * (net/enet.c) distributed as part of 4.3BSD, and code contributed
 * to Berkeley by Steven McCanne and Van Jacobson both of Lawrence
 * Berkeley Laboratory.
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
 *      @(#)bpf.h	8.1 (Berkeley) 6/10/93
 *	@(#)bpf.h	1.34 (LBL)     6/16/96
 *
 * $FreeBSD: src/sys/net/bpf.h,v 1.21.2.3 2001/08/01 00:23:13 fenner Exp $
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2006 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _NET_BPF_H_
#define _NET_BPF_H_
#include <sys/param.h>
#include <sys/appleapiopts.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/cdefs.h>
#include <stdint.h>

#ifdef PRIVATE
#include <net/if_var.h>
#include <uuid/uuid.h>

struct bpf_setup_args {
	uuid_t  bsa_uuid;
	char    bsa_ifname[IFNAMSIZ];
};
#endif /* PRIVATE */

#ifdef KERNEL
#include <sys/kernel_types.h>

#if !defined(__i386__) && !defined(__x86_64__)
#define BPF_ALIGN 1
#else /* defined(__i386__) || defined(__x86_64__) */
#define BPF_ALIGN 0
#endif /* defined(__i386__) || defined(__x86_64__) */

#if !BPF_ALIGN
#define EXTRACT_SHORT(p)        ((u_int16_t)ntohs(*(u_int16_t *)(void *)p))
#define EXTRACT_LONG(p)         (ntohl(*(u_int32_t *)(void *)p))
#else
#define EXTRACT_SHORT(p) \
	((u_int16_t)\
	        ((u_int16_t)*((u_char *)p+0)<<8|\
	         (u_int16_t)*((u_char *)p+1)<<0))
#define EXTRACT_LONG(p) \
	        ((u_int32_t)*((u_char *)p+0)<<24|\
	         (u_int32_t)*((u_char *)p+1)<<16|\
	         (u_int32_t)*((u_char *)p+2)<<8|\
	         (u_int32_t)*((u_char *)p+3)<<0)
#endif

#endif /* KERNEL */

/* BSD style release date */
#define BPF_RELEASE 199606

typedef int32_t   bpf_int32;
typedef u_int32_t bpf_u_int32;

/*
 * Alignment macros.  BPF_WORDALIGN rounds up to the next
 * even multiple of BPF_ALIGNMENT.
 */
#define BPF_ALIGNMENT sizeof(int32_t)
#define BPF_WORDALIGN(x) (((x)+(BPF_ALIGNMENT-1))&~(BPF_ALIGNMENT-1))

#define BPF_MAXINSNS 512
#define BPF_MAXBUFSIZE 0x80000
#define BPF_MINBUFSIZE 32

/*
 *  Structure for BIOCSETF.
 */
struct bpf_program {
	u_int bf_len;
	struct bpf_insn *bf_insns;
};

#ifdef KERNEL_PRIVATE
/*
 * LP64 version of bpf_program.  all pointers
 * grow when we're dealing with a 64-bit process.
 * WARNING - keep in sync with bpf_program
 */
struct bpf_program64 {
	u_int           bf_len;
	user64_addr_t   bf_insns __attribute__((aligned(8)));
};

struct bpf_program32 {
	u_int           bf_len;
	user32_addr_t   bf_insns;
};
#endif /* KERNEL_PRIVATE */

/*
 * Struct returned by BIOCGSTATS.
 */
struct bpf_stat {
	u_int bs_recv;          /* number of packets received */
	u_int bs_drop;          /* number of packets dropped */
};

/*
 * Struct return by BIOCVERSION.  This represents the version number of
 * the filter language described by the instruction encodings below.
 * bpf understands a program iff kernel_major == filter_major &&
 * kernel_minor >= filter_minor, that is, if the value returned by the
 * running kernel has the same major number and a minor number equal
 * equal to or less than the filter being downloaded.  Otherwise, the
 * results are undefined, meaning an error may be returned or packets
 * may be accepted haphazardly.
 * It has nothing to do with the source code version.
 */
struct bpf_version {
	u_short bv_major;
	u_short bv_minor;
};
#if defined(__LP64__)
#include <sys/_types/_timeval32.h>

#define BPF_TIMEVAL timeval32
#else
#define BPF_TIMEVAL timeval
#endif /* __LP64__ */
/* Current version number of filter architecture. */
#define BPF_MAJOR_VERSION 1
#define BPF_MINOR_VERSION 1

#define BIOCGBLEN       _IOR('B',102, u_int)
#define BIOCSBLEN       _IOWR('B',102, u_int)
#define BIOCSETF        _IOW('B',103, struct bpf_program)
#ifdef KERNEL_PRIVATE
#define BIOCSETF64      _IOW('B',103, struct bpf_program64)
#define BIOCSETF32      _IOW('B',103, struct bpf_program32)
#endif /* KERNEL_PRIVATE */
#define BIOCFLUSH       _IO('B',104)
#define BIOCPROMISC     _IO('B',105)
#define BIOCGDLT        _IOR('B',106, u_int)
#define BIOCGETIF       _IOR('B',107, struct ifreq)
#define BIOCSETIF       _IOW('B',108, struct ifreq)
#define BIOCSRTIMEOUT   _IOW('B',109, struct timeval)
#ifdef KERNEL_PRIVATE
#define BIOCSRTIMEOUT64 _IOW('B',109, struct user64_timeval)
#define BIOCSRTIMEOUT32 _IOW('B',109, struct user32_timeval)
#endif /* KERNEL_PRIVATE */
#define BIOCGRTIMEOUT   _IOR('B',110, struct timeval)
#ifdef KERNEL_PRIVATE
#define BIOCGRTIMEOUT64 _IOR('B',110, struct user64_timeval)
#define BIOCGRTIMEOUT32 _IOR('B',110, struct user32_timeval)
#endif /* KERNEL_PRIVATE */
#define BIOCGSTATS      _IOR('B',111, struct bpf_stat)
#define BIOCIMMEDIATE   _IOW('B',112, u_int)
#define BIOCVERSION     _IOR('B',113, struct bpf_version)
#define BIOCGRSIG       _IOR('B',114, u_int)
#define BIOCSRSIG       _IOW('B',115, u_int)
#define BIOCGHDRCMPLT   _IOR('B',116, u_int)
#define BIOCSHDRCMPLT   _IOW('B',117, u_int)
#define BIOCGSEESENT    _IOR('B',118, u_int)
#define BIOCSSEESENT    _IOW('B',119, u_int)
#define BIOCSDLT        _IOW('B',120, u_int)
#define BIOCGDLTLIST    _IOWR('B',121, struct bpf_dltlist)
#ifdef PRIVATE
#define BIOCGETTC       _IOR('B', 122, int)
#define BIOCSETTC       _IOW('B', 123, int)
#define BIOCSEXTHDR     _IOW('B', 124, u_int)
#define BIOCGIFATTACHCOUNT      _IOWR('B', 125, struct ifreq)
#endif /* PRIVATE */
#define BIOCSETFNR      _IOW('B', 126, struct bpf_program)
#ifdef KERNEL_PRIVATE
#define BIOCSETFNR64    _IOW('B',126, struct bpf_program64)
#define BIOCSETFNR32    _IOW('B',126, struct bpf_program32)
#endif /* KERNEL_PRIVATE */
#ifdef PRIVATE
#define BIOCGWANTPKTAP  _IOR('B', 127, u_int)
#define BIOCSWANTPKTAP  _IOWR('B', 127, u_int)
#define BIOCSHEADDROP   _IOW('B', 128, int)
#define BIOCGHEADDROP   _IOR('B', 128, int)
#define BIOCSTRUNCATE   _IOW('B', 129, u_int)
#define BIOCGETUUID     _IOR('B', 130, uuid_t)
#define BIOCSETUP       _IOW('B', 131, struct bpf_setup_args)
#define BIOCSPKTHDRV2   _IOW('B', 132, int)
#define BIOCGPKTHDRV2   _IOW('B', 133, int)
#endif /* PRIVATE */
/*
 * Structure prepended to each packet.
 */
struct bpf_hdr {
	struct BPF_TIMEVAL bh_tstamp;   /* time stamp */
	bpf_u_int32     bh_caplen;      /* length of captured portion */
	bpf_u_int32     bh_datalen;     /* original length of packet */
	u_short         bh_hdrlen;      /* length of bpf header (this struct
	                                 *  plus alignment padding) */
};
#ifdef KERNEL
/*
 * Because the structure above is not a multiple of 4 bytes, some compilers
 * will insist on inserting padding; hence, sizeof(struct bpf_hdr) won't work.
 * Only the kernel needs to know about it; applications use bh_hdrlen.
 */
#define SIZEOF_BPF_HDR  (sizeof(struct bpf_hdr) <= 20 ? 18 : \
    sizeof(struct bpf_hdr))
#endif
#ifdef PRIVATE
/*
 * This structure must be a multiple of 4 bytes.
 * It includes padding and spare fields that we can use later if desired.
 */
struct bpf_hdr_ext {
	struct BPF_TIMEVAL bh_tstamp;   /* time stamp */
	bpf_u_int32     bh_caplen;      /* length of captured portion */
	bpf_u_int32     bh_datalen;     /* original length of packet */
	u_short         bh_hdrlen;      /* length of bpf header */
	u_short         bh_flags;
#define BPF_HDR_EXT_FLAGS_DIR_IN        0x0000
#define BPF_HDR_EXT_FLAGS_DIR_OUT       0x0001
	pid_t           bh_pid;         /* process PID */
	char            bh_comm[MAXCOMLEN + 1]; /* process command */
	u_char          _bh_pad2[1];
	u_char          bh_pktflags;
#define BPF_PKTFLAGS_TCP_REXMT  0x0001
#define BPF_PKTFLAGS_START_SEQ  0x0002
#define BPF_PKTFLAGS_LAST_PKT   0x0004
	u_char          bh_proto;       /* kernel reserved; 0 in userland */
	bpf_u_int32     bh_svc;         /* service class */
	bpf_u_int32     bh_flowid;      /* kernel reserved; 0 in userland */
	bpf_u_int32     bh_unsent_bytes; /* unsent bytes at interface */
	bpf_u_int32     bh_unsent_snd; /* unsent bytes at socket buffer */
};

#define BPF_CONTROL_NAME        "com.apple.net.bpf"

struct bpf_mtag {
	char            bt_comm[MAXCOMLEN];
	pid_t           bt_pid;
	bpf_u_int32     bt_svc;
	unsigned char   bt_direction;
#define BPF_MTAG_DIR_IN         0
#define BPF_MTAG_DIR_OUT        1
};

#endif /* PRIVATE */

/*
 * Data-link level type codes.
 */
#define DLT_NULL        0       /* no link-layer encapsulation */
#define DLT_EN10MB      1       /* Ethernet (10Mb) */
#define DLT_EN3MB       2       /* Experimental Ethernet (3Mb) */
#define DLT_AX25        3       /* Amateur Radio AX.25 */
#define DLT_PRONET      4       /* Proteon ProNET Token Ring */
#define DLT_CHAOS       5       /* Chaos */
#define DLT_IEEE802     6       /* IEEE 802 Networks */
#define DLT_ARCNET      7       /* ARCNET */
#define DLT_SLIP        8       /* Serial Line IP */
#define DLT_PPP         9       /* Point-to-point Protocol */
#define DLT_FDDI        10      /* FDDI */
#define DLT_ATM_RFC1483 11      /* LLC/SNAP encapsulated atm */
#define DLT_RAW         12      /* raw IP */

/*
 * These are values from BSD/OS's "bpf.h".
 * These are not the same as the values from the traditional libpcap
 * "bpf.h"; however, these values shouldn't be generated by any
 * OS other than BSD/OS, so the correct values to use here are the
 * BSD/OS values.
 *
 * Platforms that have already assigned these values to other
 * DLT_ codes, however, should give these codes the values
 * from that platform, so that programs that use these codes will
 * continue to compile - even though they won't correctly read
 * files of these types.
 */
#define DLT_SLIP_BSDOS  15      /* BSD/OS Serial Line IP */
#define DLT_PPP_BSDOS   16      /* BSD/OS Point-to-point Protocol */

/*
 * 17 was used for DLT_PFLOG in OpenBSD; it no longer is.
 *
 * It was DLT_LANE8023 in SuSE 6.3, so we defined LINKTYPE_PFLOG
 * as 117 so that pflog captures would use a link-layer header type
 * value that didn't collide with any other values.  On all
 * platforms other than OpenBSD, we defined DLT_PFLOG as 117,
 * and we mapped between LINKTYPE_PFLOG and DLT_PFLOG.
 *
 * OpenBSD eventually switched to using 117 for DLT_PFLOG as well.
 *
 * Don't use 17 for anything else.
 */

/*
 * 18 is used for DLT_PFSYNC in OpenBSD, NetBSD, DragonFly BSD and
 * Mac OS X; don't use it for anything else.  (FreeBSD uses 121,
 * which collides with DLT_HHDLC, even though it doesn't use 18
 * for anything and doesn't appear to have ever used it for anything.)
 *
 * We define it as 18 on those platforms; it is, unfortunately, used
 * for DLT_CIP in Suse 6.3, so we don't define it as DLT_PFSYNC
 * in general.  As the packet format for it, like that for
 * DLT_PFLOG, is not only OS-dependent but OS-version-dependent,
 * we don't support printing it in tcpdump except on OSes that
 * have the relevant header files, so it's not that useful on
 * other platforms.
 */
#define DLT_PFSYNC      18      /* Packet filter state syncing */

#define DLT_ATM_CLIP    19      /* Linux Classical-IP over ATM */

/*
 * These values are defined by NetBSD; other platforms should refrain from
 * using them for other purposes, so that NetBSD savefiles with link
 * types of 50 or 51 can be read as this type on all platforms.
 */
#define DLT_PPP_SERIAL  50      /* PPP over serial with HDLC encapsulation */
#define DLT_PPP_ETHER   51      /* PPP over Ethernet */

/*
 * The Axent Raptor firewall - now the Symantec Enterprise Firewall - uses
 * a link-layer type of 99 for the tcpdump it supplies.  The link-layer
 * header has 6 bytes of unknown data, something that appears to be an
 * Ethernet type, and 36 bytes that appear to be 0 in at least one capture
 * I've seen.
 */
#define DLT_SYMANTEC_FIREWALL   99

/*
 * Values between 100 and 103 are used in capture file headers as
 * link-layer header type LINKTYPE_ values corresponding to DLT_ types
 * that differ between platforms; don't use those values for new DLT_
 * new types.
 */

/*
 * Values starting with 104 are used for newly-assigned link-layer
 * header type values; for those link-layer header types, the DLT_
 * value returned by pcap_datalink() and passed to pcap_open_dead(),
 * and the LINKTYPE_ value that appears in capture files, are the
 * same.
 *
 * DLT_MATCHING_MIN is the lowest such value; DLT_MATCHING_MAX is
 * the highest such value.
 */
#define DLT_MATCHING_MIN        104

/*
 * This value was defined by libpcap 0.5; platforms that have defined
 * it with a different value should define it here with that value -
 * a link type of 104 in a save file will be mapped to DLT_C_HDLC,
 * whatever value that happens to be, so programs will correctly
 * handle files with that link type regardless of the value of
 * DLT_C_HDLC.
 *
 * The name DLT_C_HDLC was used by BSD/OS; we use that name for source
 * compatibility with programs written for BSD/OS.
 *
 * libpcap 0.5 defined it as DLT_CHDLC; we define DLT_CHDLC as well,
 * for source compatibility with programs written for libpcap 0.5.
 */
#define DLT_C_HDLC      104     /* Cisco HDLC */
#define DLT_CHDLC       DLT_C_HDLC

#define DLT_IEEE802_11  105     /* IEEE 802.11 wireless */

/*
 * Values between 106 and 107 are used in capture file headers as
 * link-layer types corresponding to DLT_ types that might differ
 * between platforms; don't use those values for new DLT_ new types.
 */

/*
 * Frame Relay; BSD/OS has a DLT_FR with a value of 11, but that collides
 * with other values.
 * DLT_FR and DLT_FRELAY packets start with the Q.922 Frame Relay header
 * (DLCI, etc.).
 */
#define DLT_FRELAY      107

/*
 * OpenBSD DLT_LOOP, for loopback devices; it's like DLT_NULL, except
 * that the AF_ type in the link-layer header is in network byte order.
 *
 * OpenBSD defines it as 12, but that collides with DLT_RAW, so we
 * define it as 108 here.  If OpenBSD picks up this file, it should
 * define DLT_LOOP as 12 in its version, as per the comment above -
 * and should not use 108 for any purpose.
 */
#define DLT_LOOP        108

/*
 * Values between 109 and 112 are used in capture file headers as
 * link-layer types corresponding to DLT_ types that might differ
 * between platforms; don't use those values for new DLT_ new types.
 */

/*
 * Encapsulated packets for IPsec; DLT_ENC is 13 in OpenBSD, but that's
 * DLT_SLIP_BSDOS in NetBSD, so we don't use 13 for it in OSes other
 * than OpenBSD.
 */
#define DLT_ENC 109

/*
 * This is for Linux cooked sockets.
 */
#define DLT_LINUX_SLL   113

/*
 * Apple LocalTalk hardware.
 */
#define DLT_LTALK       114

/*
 * Acorn Econet.
 */
#define DLT_ECONET      115

/*
 * Reserved for use with OpenBSD ipfilter.
 */
#define DLT_IPFILTER    116

/*
 * For use in capture-file headers as a link-layer type corresponding
 * to OpenBSD PF (Packet Filter) log.
 */
#define DLT_PFLOG       117

/*
 * Registered for Cisco-internal use.
 */
#define DLT_CISCO_IOS   118

/*
 * Reserved for 802.11 cards using the Prism II chips, with a link-layer
 * header including Prism monitor mode information plus an 802.11
 * header.
 */
#define DLT_PRISM_HEADER        119

/*
 * Reserved for Aironet 802.11 cards, with an Aironet link-layer header
 * (see Doug Ambrisko's FreeBSD patches).
 */
#define DLT_AIRONET_HEADER      120

/*
 * Reserved for Siemens HiPath HDLC. XXX
 */
#define DLT_HHDLC       121

/*
 * Reserved for RFC 2625 IP-over-Fibre Channel.
 */
#define DLT_IP_OVER_FC  122

/*
 * Reserved for Full Frontal ATM on Solaris.
 */
#define DLT_SUNATM      123

/*
 * Reserved as per request from Kent Dahlgren <kent@praesum.com>
 * for private use.
 */
#define DLT_RIO         124     /* RapidIO */
#define DLT_PCI_EXP     125     /* PCI Express */
#define DLT_AURORA      126     /* Xilinx Aurora link layer */

/*
 * BSD header for 802.11 plus a number of bits of link-layer information
 * including radio information.
 */
#ifndef DLT_IEEE802_11_RADIO
#define DLT_IEEE802_11_RADIO    127
#endif

/*
 * Reserved for TZSP encapsulation.
 */
#define DLT_TZSP                128     /* Tazmen Sniffer Protocol */

/*
 * Reserved for Linux ARCNET.
 */
#define DLT_ARCNET_LINUX        129

/*
 * Juniper-private data link types.
 */
#define DLT_JUNIPER_MLPPP       130
#define DLT_JUNIPER_MLFR        131
#define DLT_JUNIPER_ES          132
#define DLT_JUNIPER_GGSN        133
#define DLT_JUNIPER_MFR         134
#define DLT_JUNIPER_ATM2        135
#define DLT_JUNIPER_SERVICES    136
#define DLT_JUNIPER_ATM1        137

/*
 * Apple IP-over-IEEE 1394, as per a request from Dieter Siegmund
 * <dieter@apple.com>.  The header that's presented is an Ethernet-like
 * header:
 *
 *	#define FIREWIRE_EUI64_LEN	8
 *	struct firewire_header {
 *		u_char  firewire_dhost[FIREWIRE_EUI64_LEN];
 *		u_char  firewire_shost[FIREWIRE_EUI64_LEN];
 *		u_short firewire_type;
 *	};
 *
 * with "firewire_type" being an Ethernet type value, rather than,
 * for example, raw GASP frames being handed up.
 */
#define DLT_APPLE_IP_OVER_IEEE1394      138

/*
 * Various SS7 encapsulations, as per a request from Jeff Morriss
 * <jeff.morriss[AT]ulticom.com> and subsequent discussions.
 */
#define DLT_MTP2_WITH_PHDR      139     /* pseudo-header with various info, followed by MTP2 */
#define DLT_MTP2                140     /* MTP2, without pseudo-header */
#define DLT_MTP3                141     /* MTP3, without pseudo-header or MTP2 */
#define DLT_SCCP                142     /* SCCP, without pseudo-header or MTP2 or MTP3 */

/*
 * Reserved for DOCSIS.
 */
#define DLT_DOCSIS      143

/*
 * Reserved for Linux IrDA.
 */
#define DLT_LINUX_IRDA  144

/*
 * Reserved for IBM SP switch and IBM Next Federation switch.
 */
#define DLT_IBM_SP      145
#define DLT_IBM_SN      146

/*
 * Reserved for private use.  If you have some link-layer header type
 * that you want to use within your organization, with the capture files
 * using that link-layer header type not ever be sent outside your
 * organization, you can use these values.
 *
 * No libpcap release will use these for any purpose, nor will any
 * tcpdump release use them, either.
 *
 * Do *NOT* use these in capture files that you expect anybody not using
 * your private versions of capture-file-reading tools to read; in
 * particular, do *NOT* use them in products, otherwise you may find that
 * people won't be able to use tcpdump, or snort, or Ethereal, or... to
 * read capture files from your firewall/intrusion detection/traffic
 * monitoring/etc. appliance, or whatever product uses that DLT_ value,
 * and you may also find that the developers of those applications will
 * not accept patches to let them read those files.
 *
 * Also, do not use them if somebody might send you a capture using them
 * for *their* private type and tools using them for *your* private type
 * would have to read them.
 *
 * Instead, ask "tcpdump-workers@tcpdump.org" for a new DLT_ value,
 * as per the comment above, and use the type you're given.
 */
#define DLT_USER0               147
#define DLT_USER1               148
#define DLT_USER2               149
#define DLT_USER3               150
#define DLT_USER4               151
#define DLT_USER5               152
#define DLT_USER6               153
#define DLT_USER7               154
#define DLT_USER8               155
#define DLT_USER9               156
#define DLT_USER10              157
#define DLT_USER11              158
#define DLT_USER12              159
#define DLT_USER13              160
#define DLT_USER14              161
#define DLT_USER15              162

#ifdef PRIVATE
/*
 * For Apple private usage
 */
#define DLT_USER0_APPLE_INTERNAL        DLT_USER0       /* rdar://12019509 */
#define DLT_USER1_APPLE_INTERNAL        DLT_USER1       /* rdar://12019509 */
#define DLT_PKTAP                       DLT_USER2       /* rdar://11779467 */
#define DLT_USER3_APPLE_INTERNAL        DLT_USER3       /* rdar://19614531 */
#define DLT_USER4_APPLE_INTERNAL        DLT_USER4       /* rdar://19614531 */
#endif /* PRIVATE */

/*
 * For future use with 802.11 captures - defined by AbsoluteValue
 * Systems to store a number of bits of link-layer information
 * including radio information:
 *
 *	http://www.shaftnet.org/~pizza/software/capturefrm.txt
 *
 * but it might be used by some non-AVS drivers now or in the
 * future.
 */
#define DLT_IEEE802_11_RADIO_AVS 163    /* 802.11 plus AVS radio header */

/*
 * Juniper-private data link type, as per request from
 * Hannes Gredler <hannes@juniper.net>.  The DLT_s are used
 * for passing on chassis-internal metainformation such as
 * QOS profiles, etc..
 */
#define DLT_JUNIPER_MONITOR     164

/*
 * Reserved for BACnet MS/TP.
 */
#define DLT_BACNET_MS_TP        165

/*
 * Another PPP variant as per request from Karsten Keil <kkeil@suse.de>.
 *
 * This is used in some OSes to allow a kernel socket filter to distinguish
 * between incoming and outgoing packets, on a socket intended to
 * supply pppd with outgoing packets so it can do dial-on-demand and
 * hangup-on-lack-of-demand; incoming packets are filtered out so they
 * don't cause pppd to hold the connection up (you don't want random
 * input packets such as port scans, packets from old lost connections,
 * etc. to force the connection to stay up).
 *
 * The first byte of the PPP header (0xff03) is modified to accomodate
 * the direction - 0x00 = IN, 0x01 = OUT.
 */
#define DLT_PPP_PPPD            166

/*
 * Names for backwards compatibility with older versions of some PPP
 * software; new software should use DLT_PPP_PPPD.
 */
#define DLT_PPP_WITH_DIRECTION  DLT_PPP_PPPD
#define DLT_LINUX_PPP_WITHDIRECTION     DLT_PPP_PPPD

/*
 * Juniper-private data link type, as per request from
 * Hannes Gredler <hannes@juniper.net>.  The DLT_s are used
 * for passing on chassis-internal metainformation such as
 * QOS profiles, cookies, etc..
 */
#define DLT_JUNIPER_PPPOE       167
#define DLT_JUNIPER_PPPOE_ATM   168

#define DLT_GPRS_LLC            169     /* GPRS LLC */
#define DLT_GPF_T               170     /* GPF-T (ITU-T G.7041/Y.1303) */
#define DLT_GPF_F               171     /* GPF-F (ITU-T G.7041/Y.1303) */

/*
 * Requested by Oolan Zimmer <oz@gcom.com> for use in Gcom's T1/E1 line
 * monitoring equipment.
 */
#define DLT_GCOM_T1E1           172
#define DLT_GCOM_SERIAL         173

/*
 * Juniper-private data link type, as per request from
 * Hannes Gredler <hannes@juniper.net>.  The DLT_ is used
 * for internal communication to Physical Interface Cards (PIC)
 */
#define DLT_JUNIPER_PIC_PEER    174

/*
 * Link types requested by Gregor Maier <gregor@endace.com> of Endace
 * Measurement Systems.  They add an ERF header (see
 * http://www.endace.com/support/EndaceRecordFormat.pdf) in front of
 * the link-layer header.
 */
#define DLT_ERF_ETH             175     /* Ethernet */
#define DLT_ERF_POS             176     /* Packet-over-SONET */

/*
 * Requested by Daniele Orlandi <daniele@orlandi.com> for raw LAPD
 * for vISDN (http://www.orlandi.com/visdn/).  Its link-layer header
 * includes additional information before the LAPD header, so it's
 * not necessarily a generic LAPD header.
 */
#define DLT_LINUX_LAPD          177

/*
 * Juniper-private data link type, as per request from
 * Hannes Gredler <hannes@juniper.net>.
 * The DLT_ are used for prepending meta-information
 * like interface index, interface name
 * before standard Ethernet, PPP, Frelay & C-HDLC Frames
 */
#define DLT_JUNIPER_ETHER       178
#define DLT_JUNIPER_PPP         179
#define DLT_JUNIPER_FRELAY      180
#define DLT_JUNIPER_CHDLC       181

/*
 * Multi Link Frame Relay (FRF.16)
 */
#define DLT_MFR                 182

/*
 * Juniper-private data link type, as per request from
 * Hannes Gredler <hannes@juniper.net>.
 * The DLT_ is used for internal communication with a
 * voice Adapter Card (PIC)
 */
#define DLT_JUNIPER_VP          183

/*
 * Arinc 429 frames.
 * DLT_ requested by Gianluca Varenni <gianluca.varenni@cacetech.com>.
 * Every frame contains a 32bit A429 label.
 * More documentation on Arinc 429 can be found at
 * http://www.condoreng.com/support/downloads/tutorials/ARINCTutorial.pdf
 */
#define DLT_A429                184

/*
 * Arinc 653 Interpartition Communication messages.
 * DLT_ requested by Gianluca Varenni <gianluca.varenni@cacetech.com>.
 * Please refer to the A653-1 standard for more information.
 */
#define DLT_A653_ICM            185

/*
 * USB packets, beginning with a USB setup header; requested by
 * Paolo Abeni <paolo.abeni@email.it>.
 */
#define DLT_USB                 186

/*
 * Bluetooth HCI UART transport layer (part H:4); requested by
 * Paolo Abeni.
 */
#define DLT_BLUETOOTH_HCI_H4    187

/*
 * IEEE 802.16 MAC Common Part Sublayer; requested by Maria Cruz
 * <cruz_petagay@bah.com>.
 */
#define DLT_IEEE802_16_MAC_CPS  188

/*
 * USB packets, beginning with a Linux USB header; requested by
 * Paolo Abeni <paolo.abeni@email.it>.
 */
#define DLT_USB_LINUX           189

/*
 * Controller Area Network (CAN) v. 2.0B packets.
 * DLT_ requested by Gianluca Varenni <gianluca.varenni@cacetech.com>.
 * Used to dump CAN packets coming from a CAN Vector board.
 * More documentation on the CAN v2.0B frames can be found at
 * http://www.can-cia.org/downloads/?269
 */
#define DLT_CAN20B              190

/*
 * IEEE 802.15.4, with address fields padded, as is done by Linux
 * drivers; requested by Juergen Schimmer.
 */
#define DLT_IEEE802_15_4_LINUX  191

/*
 * Per Packet Information encapsulated packets.
 * DLT_ requested by Gianluca Varenni <gianluca.varenni@cacetech.com>.
 */
#define DLT_PPI                 192

/*
 * Header for 802.16 MAC Common Part Sublayer plus a radiotap radio header;
 * requested by Charles Clancy.
 */
#define DLT_IEEE802_16_MAC_CPS_RADIO    193

/*
 * Juniper-private data link type, as per request from
 * Hannes Gredler <hannes@juniper.net>.
 * The DLT_ is used for internal communication with a
 * integrated service module (ISM).
 */
#define DLT_JUNIPER_ISM         194

/*
 * IEEE 802.15.4, exactly as it appears in the spec (no padding, no
 * nothing); requested by Mikko Saarnivala <mikko.saarnivala@sensinode.com>.
 */
#define DLT_IEEE802_15_4        195

/*
 * Various link-layer types, with a pseudo-header, for SITA
 * (http://www.sita.aero/); requested by Fulko Hew (fulko.hew@gmail.com).
 */
#define DLT_SITA                196

/*
 * Various link-layer types, with a pseudo-header, for Endace DAG cards;
 * encapsulates Endace ERF records.  Requested by Stephen Donnelly
 * <stephen@endace.com>.
 */
#define DLT_ERF                 197

/*
 * Special header prepended to Ethernet packets when capturing from a
 * u10 Networks board.  Requested by Phil Mulholland
 * <phil@u10networks.com>.
 */
#define DLT_RAIF1               198

/*
 * IPMB packet for IPMI, beginning with the I2C slave address, followed
 * by the netFn and LUN, etc..  Requested by Chanthy Toeung
 * <chanthy.toeung@ca.kontron.com>.
 */
#define DLT_IPMB                199

/*
 * Juniper-private data link type, as per request from
 * Hannes Gredler <hannes@juniper.net>.
 * The DLT_ is used for capturing data on a secure tunnel interface.
 */
#define DLT_JUNIPER_ST          200

/*
 * Bluetooth HCI UART transport layer (part H:4), with pseudo-header
 * that includes direction information; requested by Paolo Abeni.
 */
#define DLT_BLUETOOTH_HCI_H4_WITH_PHDR  201

/*
 * AX.25 packet with a 1-byte KISS header; see
 *
 *      http://www.ax25.net/kiss.htm
 *
 * as per Richard Stearn <richard@rns-stearn.demon.co.uk>.
 */
#define DLT_AX25_KISS           202

/*
 * LAPD packets from an ISDN channel, starting with the address field,
 * with no pseudo-header.
 * Requested by Varuna De Silva <varunax@gmail.com>.
 */
#define DLT_LAPD                203

/*
 * Variants of various link-layer headers, with a one-byte direction
 * pseudo-header prepended - zero means "received by this host",
 * non-zero (any non-zero value) means "sent by this host" - as per
 * Will Barker <w.barker@zen.co.uk>.
 */
#define DLT_PPP_WITH_DIR        204     /* PPP - don't confuse with DLT_PPP_WITH_DIRECTION */
#define DLT_C_HDLC_WITH_DIR     205     /* Cisco HDLC */
#define DLT_FRELAY_WITH_DIR     206     /* Frame Relay */
#define DLT_LAPB_WITH_DIR       207     /* LAPB */

/*
 * 208 is reserved for an as-yet-unspecified proprietary link-layer
 * type, as requested by Will Barker.
 */

/*
 * IPMB with a Linux-specific pseudo-header; as requested by Alexey Neyman
 * <avn@pigeonpoint.com>.
 */
#define DLT_IPMB_LINUX          209

/*
 * FlexRay automotive bus - http://www.flexray.com/ - as requested
 * by Hannes Kaelber <hannes.kaelber@x2e.de>.
 */
#define DLT_FLEXRAY             210

/*
 * Media Oriented Systems Transport (MOST) bus for multimedia
 * transport - http://www.mostcooperation.com/ - as requested
 * by Hannes Kaelber <hannes.kaelber@x2e.de>.
 */
#define DLT_MOST                211

/*
 * Local Interconnect Network (LIN) bus for vehicle networks -
 * http://www.lin-subbus.org/ - as requested by Hannes Kaelber
 * <hannes.kaelber@x2e.de>.
 */
#define DLT_LIN                 212

/*
 * X2E-private data link type used for serial line capture,
 * as requested by Hannes Kaelber <hannes.kaelber@x2e.de>.
 */
#define DLT_X2E_SERIAL          213

/*
 * X2E-private data link type used for the Xoraya data logger
 * family, as requested by Hannes Kaelber <hannes.kaelber@x2e.de>.
 */
#define DLT_X2E_XORAYA          214

/*
 * IEEE 802.15.4, exactly as it appears in the spec (no padding, no
 * nothing), but with the PHY-level data for non-ASK PHYs (4 octets
 * of 0 as preamble, one octet of SFD, one octet of frame length+
 * reserved bit, and then the MAC-layer data, starting with the
 * frame control field).
 *
 * Requested by Max Filippov <jcmvbkbc@gmail.com>.
 */
#define DLT_IEEE802_15_4_NONASK_PHY     215

/*
 * David Gibson <david@gibson.dropbear.id.au> requested this for
 * captures from the Linux kernel /dev/input/eventN devices. This
 * is used to communicate keystrokes and mouse movements from the
 * Linux kernel to display systems, such as Xorg.
 */
#define DLT_LINUX_EVDEV         216

/*
 * GSM Um and Abis interfaces, preceded by a "gsmtap" header.
 *
 * Requested by Harald Welte <laforge@gnumonks.org>.
 */
#define DLT_GSMTAP_UM           217
#define DLT_GSMTAP_ABIS         218

/*
 * MPLS, with an MPLS label as the link-layer header.
 * Requested by Michele Marchetto <michele@openbsd.org> on behalf
 * of OpenBSD.
 */
#define DLT_MPLS                219

/*
 * USB packets, beginning with a Linux USB header, with the USB header
 * padded to 64 bytes; required for memory-mapped access.
 */
#define DLT_USB_LINUX_MMAPPED   220

/*
 * DECT packets, with a pseudo-header; requested by
 * Matthias Wenzel <tcpdump@mazzoo.de>.
 */
#define DLT_DECT                221

/*
 * From: "Lidwa, Eric (GSFC-582.0)[SGT INC]" <eric.lidwa-1@nasa.gov>
 * Date: Mon, 11 May 2009 11:18:30 -0500
 *
 * DLT_AOS. We need it for AOS Space Data Link Protocol.
 *   I have already written dissectors for but need an OK from
 *   legal before I can submit a patch.
 *
 */
#define DLT_AOS                 222

/*
 * Wireless HART (Highway Addressable Remote Transducer)
 * From the HART Communication Foundation
 * IES/PAS 62591
 *
 * Requested by Sam Roberts <vieuxtech@gmail.com>.
 */
#define DLT_WIHART              223

/*
 * Fibre Channel FC-2 frames, beginning with a Frame_Header.
 * Requested by Kahou Lei <kahou82@gmail.com>.
 */
#define DLT_FC_2                224

/*
 * Fibre Channel FC-2 frames, beginning with an encoding of the
 * SOF, and ending with an encoding of the EOF.
 *
 * The encodings represent the frame delimiters as 4-byte sequences
 * representing the corresponding ordered sets, with K28.5
 * represented as 0xBC, and the D symbols as the corresponding
 * byte values; for example, SOFi2, which is K28.5 - D21.5 - D1.2 - D21.2,
 * is represented as 0xBC 0xB5 0x55 0x55.
 *
 * Requested by Kahou Lei <kahou82@gmail.com>.
 */
#define DLT_FC_2_WITH_FRAME_DELIMS      225

/*
 * Solaris ipnet pseudo-header; requested by Darren Reed <Darren.Reed@Sun.COM>.
 *
 * The pseudo-header starts with a one-byte version number; for version 2,
 * the pseudo-header is:
 *
 * struct dl_ipnetinfo {
 *     u_int8_t   dli_version;
 *     u_int8_t   dli_family;
 *     u_int16_t  dli_htype;
 *     u_int32_t  dli_pktlen;
 *     u_int32_t  dli_ifindex;
 *     u_int32_t  dli_grifindex;
 *     u_int32_t  dli_zsrc;
 *     u_int32_t  dli_zdst;
 * };
 *
 * dli_version is 2 for the current version of the pseudo-header.
 *
 * dli_family is a Solaris address family value, so it's 2 for IPv4
 * and 26 for IPv6.
 *
 * dli_htype is a "hook type" - 0 for incoming packets, 1 for outgoing
 * packets, and 2 for packets arriving from another zone on the same
 * machine.
 *
 * dli_pktlen is the length of the packet data following the pseudo-header
 * (so the captured length minus dli_pktlen is the length of the
 * pseudo-header, assuming the entire pseudo-header was captured).
 *
 * dli_ifindex is the interface index of the interface on which the
 * packet arrived.
 *
 * dli_grifindex is the group interface index number (for IPMP interfaces).
 *
 * dli_zsrc is the zone identifier for the source of the packet.
 *
 * dli_zdst is the zone identifier for the destination of the packet.
 *
 * A zone number of 0 is the global zone; a zone number of 0xffffffff
 * means that the packet arrived from another host on the network, not
 * from another zone on the same machine.
 *
 * An IPv4 or IPv6 datagram follows the pseudo-header; dli_family indicates
 * which of those it is.
 */
#define DLT_IPNET               226

/*
 * CAN (Controller Area Network) frames, with a pseudo-header as supplied
 * by Linux SocketCAN.  See Documentation/networking/can.txt in the Linux
 * source.
 *
 * Requested by Felix Obenhuber <felix@obenhuber.de>.
 */
#define DLT_CAN_SOCKETCAN       227

/*
 * Raw IPv4/IPv6; different from DLT_RAW in that the DLT_ value specifies
 * whether it's v4 or v6.  Requested by Darren Reed <Darren.Reed@Sun.COM>.
 */
#define DLT_IPV4                228
#define DLT_IPV6                229

/*
 * IEEE 802.15.4, exactly as it appears in the spec (no padding, no
 * nothing), and with no FCS at the end of the frame; requested by
 * Jon Smirl <jonsmirl@gmail.com>.
 */
#define DLT_IEEE802_15_4_NOFCS  230

/*
 * Raw D-Bus:
 *
 *	http://www.freedesktop.org/wiki/Software/dbus
 *
 * messages:
 *
 *	http://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-messages
 *
 * starting with the endianness flag, followed by the message type, etc.,
 * but without the authentication handshake before the message sequence:
 *
 *	http://dbus.freedesktop.org/doc/dbus-specification.html#auth-protocol
 *
 * Requested by Martin Vidner <martin@vidner.net>.
 */
#define DLT_DBUS                231

/*
 * Juniper-private data link type, as per request from
 * Hannes Gredler <hannes@juniper.net>.
 */
#define DLT_JUNIPER_VS                  232
#define DLT_JUNIPER_SRX_E2E             233
#define DLT_JUNIPER_FIBRECHANNEL        234

/*
 * DVB-CI (DVB Common Interface for communication between a PC Card
 * module and a DVB receiver).  See
 *
 *	http://www.kaiser.cx/pcap-dvbci.html
 *
 * for the specification.
 *
 * Requested by Martin Kaiser <martin@kaiser.cx>.
 */
#define DLT_DVB_CI              235

/*
 * Variant of 3GPP TS 27.010 multiplexing protocol (similar to, but
 * *not* the same as, 27.010).  Requested by Hans-Christoph Schemmel
 * <hans-christoph.schemmel@cinterion.com>.
 */
#define DLT_MUX27010            236

/*
 * STANAG 5066 D_PDUs.  Requested by M. Baris Demiray
 * <barisdemiray@gmail.com>.
 */
#define DLT_STANAG_5066_D_PDU   237

/*
 * Juniper-private data link type, as per request from
 * Hannes Gredler <hannes@juniper.net>.
 */
#define DLT_JUNIPER_ATM_CEMIC   238

/*
 * NetFilter LOG messages
 * (payload of netlink NFNL_SUBSYS_ULOG/NFULNL_MSG_PACKET packets)
 *
 * Requested by Jakub Zawadzki <darkjames-ws@darkjames.pl>
 */
#define DLT_NFLOG               239

/*
 * Hilscher Gesellschaft fuer Systemautomation mbH link-layer type
 * for Ethernet packets with a 4-byte pseudo-header and always
 * with the payload including the FCS, as supplied by their
 * netANALYZER hardware and software.
 *
 * Requested by Holger P. Frommer <HPfrommer@hilscher.com>
 */
#define DLT_NETANALYZER         240

/*
 * Hilscher Gesellschaft fuer Systemautomation mbH link-layer type
 * for Ethernet packets with a 4-byte pseudo-header and FCS and
 * with the Ethernet header preceded by 7 bytes of preamble and
 * 1 byte of SFD, as supplied by their netANALYZER hardware and
 * software.
 *
 * Requested by Holger P. Frommer <HPfrommer@hilscher.com>
 */
#define DLT_NETANALYZER_TRANSPARENT     241

/*
 * IP-over-Infiniband, as specified by RFC 4391.
 *
 * Requested by Petr Sumbera <petr.sumbera@oracle.com>.
 */
#define DLT_IPOIB               242

/*
 * MPEG-2 transport stream (ISO 13818-1/ITU-T H.222.0).
 *
 * Requested by Guy Martin <gmsoft@tuxicoman.be>.
 */
#define DLT_MPEG_2_TS           243

/*
 * ng4T GmbH's UMTS Iub/Iur-over-ATM and Iub/Iur-over-IP format as
 * used by their ng40 protocol tester.
 *
 * Requested by Jens Grimmer <jens.grimmer@ng4t.com>.
 */
#define DLT_NG40                244

/*
 * Pseudo-header giving adapter number and flags, followed by an NFC
 * (Near-Field Communications) Logical Link Control Protocol (LLCP) PDU,
 * as specified by NFC Forum Logical Link Control Protocol Technical
 * Specification LLCP 1.1.
 *
 * Requested by Mike Wakerly <mikey@google.com>.
 */
#define DLT_NFC_LLCP            245

/*
 * USB packets, beginning with a Darwin (macOS, etc.) USB header.
 */
#define DLT_USB_DARWIN          266

#define DLT_MATCHING_MAX        266     /* highest value in the "matching" range */

/*
 * The instruction encodings.
 */
/* instruction classes */
#define BPF_CLASS(code) ((code) & 0x07)
#define         BPF_LD          0x00
#define         BPF_LDX         0x01
#define         BPF_ST          0x02
#define         BPF_STX         0x03
#define         BPF_ALU         0x04
#define         BPF_JMP         0x05
#define         BPF_RET         0x06
#define         BPF_MISC        0x07

/* ld/ldx fields */
#define BPF_SIZE(code)  ((code) & 0x18)
#define         BPF_W           0x00
#define         BPF_H           0x08
#define         BPF_B           0x10
#define BPF_MODE(code)  ((code) & 0xe0)
#define         BPF_IMM         0x00
#define         BPF_ABS         0x20
#define         BPF_IND         0x40
#define         BPF_MEM         0x60
#define         BPF_LEN         0x80
#define         BPF_MSH         0xa0

/* alu/jmp fields */
#define BPF_OP(code)    ((code) & 0xf0)
#define         BPF_ADD         0x00
#define         BPF_SUB         0x10
#define         BPF_MUL         0x20
#define         BPF_DIV         0x30
#define         BPF_OR          0x40
#define         BPF_AND         0x50
#define         BPF_LSH         0x60
#define         BPF_RSH         0x70
#define         BPF_NEG         0x80
#define         BPF_JA          0x00
#define         BPF_JEQ         0x10
#define         BPF_JGT         0x20
#define         BPF_JGE         0x30
#define         BPF_JSET        0x40
#define BPF_SRC(code)   ((code) & 0x08)
#define         BPF_K           0x00
#define         BPF_X           0x08

/* ret - BPF_K and BPF_X also apply */
#define BPF_RVAL(code)  ((code) & 0x18)
#define         BPF_A           0x10

/* misc */
#define BPF_MISCOP(code) ((code) & 0xf8)
#define         BPF_TAX         0x00
#define         BPF_TXA         0x80

/*
 * The instruction data structure.
 */
struct bpf_insn {
	u_short         code;
	u_char          jt;
	u_char          jf;
	bpf_u_int32     k;
};

/*
 * Macros for insn array initializers.
 */
#define BPF_STMT(code, k) { (u_short)(code), 0, 0, k }
#define BPF_JUMP(code, k, jt, jf) { (u_short)(code), jt, jf, k }

#pragma pack(4)

/*
 * Structure to retrieve available DLTs for the interface.
 */
struct bpf_dltlist {
	u_int32_t               bfl_len;        /* number of bfd_list array */
	union {
		u_int32_t       *bflu_list;     /* array of DLTs */
		u_int64_t       bflu_pad;
	} bfl_u;
};
#define bfl_list bfl_u.bflu_list

#pragma pack()

#ifdef KERNEL_PRIVATE
#define BPF_MIN_PKT_SIZE 40
#define PORT_DNS 53
#define PORT_BOOTPS 67
#define PORT_BOOTPC 68
#define PORT_ISAKMP 500
#define PORT_ISAKMP_NATT 4500   /* rfc3948 */

/* Forward declerations */
struct ifnet;
struct mbuf;

#define BPF_PACKET_TYPE_MBUF    0

struct bpf_packet {
	int     bpfp_type;
	void *  bpfp_header;            /* optional */
	size_t  bpfp_header_length;
	union {
		struct mbuf     *bpfpu_mbuf;
		void *          bpfpu_ptr;
	} bpfp_u;
#define bpfp_mbuf       bpfp_u.bpfpu_mbuf
#define bpfp_ptr        bpfp_u.bpfpu_ptr
	size_t  bpfp_total_length;      /* length including optional header */
};

extern int      bpf_validate(const struct bpf_insn *, int);
extern void     bpfdetach(struct ifnet *);
extern void     bpfilterattach(int);
extern u_int    bpf_filter(const struct bpf_insn *, u_char *, u_int, u_int);
#endif /* KERNEL_PRIVATE */

#ifdef KERNEL
#ifndef BPF_TAP_MODE_T
#define BPF_TAP_MODE_T
/*!
 *       @enum BPF tap mode
 *       @abstract Constants defining interface families.
 *       @constant BPF_MODE_DISABLED Disable bpf.
 *       @constant BPF_MODE_INPUT Enable input only.
 *       @constant BPF_MODE_OUTPUT Enable output only.
 *       @constant BPF_MODE_INPUT_OUTPUT Enable input and output.
 */

enum {
	BPF_MODE_DISABLED       = 0,
	BPF_MODE_INPUT          = 1,
	BPF_MODE_OUTPUT         = 2,
	BPF_MODE_INPUT_OUTPUT   = 3
};
/*!
 *       @typedef bpf_tap_mode
 *       @abstract Mode for tapping. BPF_MODE_DISABLED/BPF_MODE_INPUT_OUTPUT etc.
 */
typedef u_int32_t bpf_tap_mode;
#endif /* !BPF_TAP_MODE_T */

/*!
 *       @typedef bpf_send_func
 *       @discussion bpf_send_func is called when a bpf file descriptor is
 *               used to send a raw packet on the interface. The mbuf and data
 *               link type are specified. The callback is responsible for
 *               releasing the mbuf whether or not it returns an error.
 *       @param interface The interface the packet is being sent on.
 *       @param data_link_type The data link type the bpf device is attached to.
 *       @param packet The packet to be sent.
 */
typedef errno_t (*bpf_send_func)(ifnet_t interface, u_int32_t data_link_type,
    mbuf_t packet);

/*!
 *       @typedef bpf_tap_func
 *       @discussion bpf_tap_func is called when the tap state of the
 *               interface changes. This happens when a bpf device attaches to an
 *               interface or detaches from an interface. The tap mode will join
 *               together (bit or) the modes of all bpf devices using that
 *               interface for that dlt. If you return an error from this
 *               function, the bpf device attach attempt that triggered the tap
 *               will fail. If this function was called bacuse the tap state was
 *               decreasing (tap in or out is stopping), the error will be
 *               ignored.
 *       @param interface The interface being tapped.
 *       @param data_link_type The data link type being tapped.
 *       @param direction The direction of the tap.
 */
typedef errno_t (*bpf_tap_func)(ifnet_t interface, u_int32_t data_link_type,
    bpf_tap_mode direction);

/*!
 *       @function bpfattach
 *       @discussion Registers an interface with BPF. This allows bpf devices
 *               to attach to your interface to capture packets. Your interface
 *               will be unregistered automatically when your interface is
 *               detached.
 *       @param interface The interface to register with BPF.
 *       @param data_link_type The data link type of the interface. See the
 *               DLT_* defines in bpf.h.
 *       @param header_length The length, in bytes, of the data link header.
 */
extern void  bpfattach(ifnet_t interface, u_int data_link_type,
    u_int header_length);

/*!
 *       @function bpf_attach
 *       @discussion Registers an interface with BPF. This allows bpf devices
 *               to attach to your interface to capture and transmit packets.
 *               Your interface will be unregistered automatically when your
 *               interface is detached. You may register multiple times with
 *               different data link types. An 802.11 interface would use this to
 *               allow clients to pick whether they want just an ethernet style
 *               frame or the 802.11 wireless headers as well. The first dlt you
 *               register will be considered the default. Any bpf device attaches
 *               that do not specify a data link type will use the default.
 *       @param interface The interface to register with BPF.
 *       @param data_link_type The data link type of the interface. See the
 *               DLT_* defines in bpf.h.
 *       @param header_length The length, in bytes, of the data link header.
 *       @param send See the bpf_send_func described above.
 *       @param tap See the bpf_tap_func described above.
 */
extern errno_t  bpf_attach(ifnet_t interface, u_int32_t data_link_type,
    u_int32_t header_length, bpf_send_func send, bpf_tap_func tap);

/*!
 *       @function bpf_tap_in
 *       @discussion Call this function when your interface receives a
 *               packet. This function will check if any bpf devices need a
 *               a copy of the packet.
 *       @param interface The interface the packet was received on.
 *       @param dlt The data link type of the packet.
 *       @param packet The packet received.
 *       @param header An optional pointer to a header that will be prepended.
 *       @param header_len If the header was specified, the length of the header.
 */
extern void bpf_tap_in(ifnet_t interface, u_int32_t dlt, mbuf_t packet,
    void *header, size_t header_len);

/*!
 *       @function bpf_tap_out
 *       @discussion Call this function when your interface transmits a
 *               packet. This function will check if any bpf devices need a
 *               a copy of the packet.
 *       @param interface The interface the packet was or will be transmitted on.
 *       @param dlt The data link type of the packet.
 *       @param packet The packet received.
 *       @param header An optional pointer to a header that will be prepended.
 *       @param header_len If the header was specified, the length of the header.
 */
extern void bpf_tap_out(ifnet_t interface, u_int32_t dlt, mbuf_t packet,
    void *header, size_t header_len);

#endif /* KERNEL */

/*
 * Number of scratch memory words (for BPF_LD|BPF_MEM and BPF_ST).
 */
#define BPF_MEMWORDS 16

#endif /* _NET_BPF_H_ */
