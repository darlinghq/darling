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
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)tcp.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/netinet/tcp.h,v 1.13.2.3 2001/03/01 22:08:42 jlemon Exp $
 */

#ifndef _NETINET_TCP_H_
#define _NETINET_TCP_H_
#include <sys/types.h>
#include <sys/appleapiopts.h>
#include <machine/endian.h>
#include <machine/types.h> /* __uint32_t */

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
typedef __uint32_t tcp_seq;
typedef __uint32_t tcp_cc;              /* connection count per rfc1644 */

#define tcp6_seq        tcp_seq /* for KAME src sync over BSD*'s */
#define tcp6hdr         tcphdr  /* for KAME src sync over BSD*'s */

/*
 * TCP header.
 * Per RFC 793, September, 1981.
 */
struct tcphdr {
	unsigned short  th_sport;       /* source port */
	unsigned short  th_dport;       /* destination port */
	tcp_seq th_seq;                 /* sequence number */
	tcp_seq th_ack;                 /* acknowledgement number */
#if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
	unsigned int    th_x2:4,        /* (unused) */
	    th_off:4;                   /* data offset */
#endif
#if __DARWIN_BYTE_ORDER == __DARWIN_BIG_ENDIAN
	unsigned int    th_off:4,       /* data offset */
	    th_x2:4;                    /* (unused) */
#endif
	unsigned char   th_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
#define TH_ACCEPT       (TH_FIN|TH_SYN|TH_RST|TH_ACK)

	unsigned short  th_win;         /* window */
	unsigned short  th_sum;         /* checksum */
	unsigned short  th_urp;         /* urgent pointer */
};

#define TCPOPT_EOL              0
#define TCPOPT_NOP              1
#define TCPOPT_MAXSEG           2
#define TCPOLEN_MAXSEG          4
#define TCPOPT_WINDOW           3
#define TCPOLEN_WINDOW          3
#define TCPOPT_SACK_PERMITTED   4               /* Experimental */
#define TCPOLEN_SACK_PERMITTED  2
#define TCPOPT_SACK             5               /* Experimental */
#define TCPOLEN_SACK            8               /* len of sack block */
#define TCPOPT_TIMESTAMP        8
#define TCPOLEN_TIMESTAMP       10
#define TCPOLEN_TSTAMP_APPA             (TCPOLEN_TIMESTAMP+2) /* appendix A */
#define TCPOPT_TSTAMP_HDR               \
    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_TIMESTAMP<<8|TCPOLEN_TIMESTAMP)

#define MAX_TCPOPTLEN           40      /* Absolute maximum TCP options len */

#define TCPOPT_CC               11              /* CC options: RFC-1644 */
#define TCPOPT_CCNEW            12
#define TCPOPT_CCECHO           13
#define    TCPOLEN_CC                   6
#define    TCPOLEN_CC_APPA              (TCPOLEN_CC+2)
#define    TCPOPT_CC_HDR(ccopt)         \
    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|(ccopt)<<8|TCPOLEN_CC)
#define TCPOPT_SIGNATURE                19      /* Keyed MD5: RFC 2385 */
#define    TCPOLEN_SIGNATURE            18
#if MPTCP
#define TCPOPT_MULTIPATH                30
#endif

#define TCPOPT_FASTOPEN                 34
#define TCPOLEN_FASTOPEN_REQ            2

/* Option definitions */
#define TCPOPT_SACK_PERMIT_HDR  \
(TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_SACK_PERMITTED<<8|TCPOLEN_SACK_PERMITTED)
#define TCPOPT_SACK_HDR         (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_SACK<<8)
/* Miscellaneous constants */
#define MAX_SACK_BLKS   6       /* Max # SACK blocks stored at sender side */

/*
 * A SACK option that specifies n blocks will have a length of (8*n + 2)
 * bytes, so the 40 bytes available for TCP options can specify a
 * maximum of 4 blocks.
 */

#define TCP_MAX_SACK    4       /* MAX # SACKs sent in any segment */


/*
 * Default maximum segment size for TCP.
 * With an IP MTU of 576, this is 536,
 * but 512 is probably more convenient.
 * This should be defined as MIN(512, IP_MSS - sizeof (struct tcpiphdr)).
 */
#define TCP_MSS 512

/*
 * TCP_MINMSS is defined to be 216 which is fine for the smallest
 * link MTU (256 bytes, SLIP interface) in the Internet.
 * However it is very unlikely to come across such low MTU interfaces
 * these days (anno dato 2004).
 * Probably it can be set to 512 without ill effects. But we play safe.
 * See tcp_subr.c tcp_minmss SYSCTL declaration for more comments.
 * Setting this to "0" disables the minmss check.
 */
#define TCP_MINMSS 216

/*
 * Default maximum segment size for TCP6.
 * With an IP6 MSS of 1280, this is 1220,
 * but 1024 is probably more convenient. (xxx kazu in doubt)
 * This should be defined as MIN(1024, IP6_MSS - sizeof (struct tcpip6hdr))
 */
#define TCP6_MSS        1024

#define TCP_MAXWIN      65535   /* largest value for (unscaled) window */
#define TTCP_CLIENT_SND_WND     4096    /* dflt send window for T/TCP client */

#define TCP_MAX_WINSHIFT        14      /* maximum window shift */

#define TCP_MAXHLEN     (0xf<<2)        /* max length of header in bytes */
#define TCP_MAXOLEN     (TCP_MAXHLEN - sizeof(struct tcphdr))
/* max space left for options */
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

/*
 * User-settable options (used with setsockopt).
 */
#define TCP_NODELAY             0x01    /* don't delay send to coalesce packets */
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#define TCP_MAXSEG              0x02    /* set maximum segment size */
#define TCP_NOPUSH              0x04    /* don't push last block of write */
#define TCP_NOOPT               0x08    /* don't use TCP options */
#define TCP_KEEPALIVE           0x10    /* idle time used when SO_KEEPALIVE is enabled */
#define TCP_CONNECTIONTIMEOUT   0x20    /* connection timeout */
#define PERSIST_TIMEOUT         0x40    /* time after which a connection in
	                                 *  persist timeout will terminate.
	                                 *  see draft-ananth-tcpm-persist-02.txt
	                                 */
#define TCP_RXT_CONNDROPTIME    0x80    /* time after which tcp retransmissions will be
	                                 * stopped and the connection will be dropped
	                                 */
#define TCP_RXT_FINDROP         0x100   /* when this option is set, drop a connection
	                                 * after retransmitting the FIN 3 times. It will
	                                 * prevent holding too many mbufs in socket
	                                 * buffer queues.
	                                 */
#define TCP_KEEPINTVL           0x101   /* interval between keepalives */
#define TCP_KEEPCNT             0x102   /* number of keepalives before close */
#define TCP_SENDMOREACKS        0x103   /* always ack every other packet */
#define TCP_ENABLE_ECN          0x104   /* Enable ECN on a connection */
#define TCP_FASTOPEN            0x105   /* Enable/Disable TCP Fastopen on this socket */
#define TCP_CONNECTION_INFO     0x106   /* State of TCP connection */

#ifdef PRIVATE
#define TCP_INFO                        0x200   /* retrieve tcp_info structure */
#define TCP_MEASURE_SND_BW              0x202   /* Measure sender's bandwidth for this connection */
#endif /* PRIVATE */


#define TCP_NOTSENT_LOWAT       0x201   /* Low water mark for TCP unsent data */

#ifdef PRIVATE
#define TCP_MEASURE_BW_BURST    0x203   /* Burst size to use for bandwidth measurement */
#define TCP_PEER_PID            0x204   /* Lookup pid of the process we're connected to */
#define TCP_ADAPTIVE_READ_TIMEOUT       0x205   /* Read timeout used as a multiple of RTT */
/*
 * Enable message delivery on a socket, this feature is currently unsupported and
 * is subjected to change in future.
 */
#define TCP_ENABLE_MSGS                 0x206
#define TCP_ADAPTIVE_WRITE_TIMEOUT      0x207   /* Write timeout used as a multiple of RTT */
#define TCP_NOTIMEWAIT                  0x208   /* Avoid going into time-wait */
#define TCP_DISABLE_BLACKHOLE_DETECTION 0x209   /* disable PMTU blackhole detection */
#define TCP_ECN_MODE                    0x210   /* fine grain control for A/B testing */
#define TCP_KEEPALIVE_OFFLOAD           0x211   /* offload keep alive processing to firmware */

/*
 * TCP_ECN_MODE values
 */
#define ECN_MODE_DEFAULT                0x0     /* per interface or system wide default */
#define ECN_MODE_ENABLE                 0x1     /* force enable ECN on connection */
#define ECN_MODE_DISABLE                0x2     /* force disable ECN on connection */

/*
 * TCP_NOTIFY_ACKNOWLEDGEMENT
 *
 * Application can use this socket option to get a notification when
 * data that is currently written to the socket is acknowledged. The input
 * argument given to this socket option is a marker_id that will be used for
 * returning the notification. The application can continue to write
 * data after setting the marker. There can be multiple of these events
 * outstanding on a socket at any time up to a max of TCP_MAX_NOTIFY_ACK.
 *
 * To get the completed notifications, getsockopt should be called with the
 * TCP_NOTIFY_ACKNOWLEDGEMENT with the following tcp_notify_ack_complete
 * structure as an out argument. At most TCP_MAX_NOTIFY_ACK ids will be
 * returned if they have been successfully acknowledged in each call.
 */

#define TCP_MAX_NOTIFY_ACK      10

typedef u_int32_t       tcp_notify_ack_id_t;

struct tcp_notify_ack_complete {
	u_int32_t       notify_pending; /* still pending */
	u_int32_t       notify_complete_count;
	tcp_notify_ack_id_t notify_complete_id[TCP_MAX_NOTIFY_ACK];
};

#define TCP_NOTIFY_ACKNOWLEDGEMENT      0x212   /* Notify when data is acknowledged */
#define MPTCP_SERVICE_TYPE              0x213   /* MPTCP Service type */
#define TCP_FASTOPEN_FORCE_HEURISTICS   0x214   /* Make sure TFO-heuristics never get disabled */

#define MPTCP_SVCTYPE_HANDOVER          0 /* Default 0 */
#define MPTCP_SVCTYPE_INTERACTIVE       1
#define MPTCP_SVCTYPE_AGGREGATE         2
#define MPTCP_SVCTYPE_TARGET_BASED      3
#define MPTCP_SVCTYPE_MAX               4
/*
 * Specify minimum time in seconds before which an established
 * TCP connection will not be dropped when there is no response from the
 * peer
 */
#define TCP_RXT_MINIMUM_TIMEOUT         0x215

#define TCP_RXT_MINIMUM_TIMEOUT_LIMIT   (5 * 60) /* Limit is 5 minutes */

#define MPTCP_ALTERNATE_PORT            0x216
#define MPTCP_FORCE_ENABLE              0x217
#define TCP_FASTOPEN_FORCE_ENABLE       0x218
#define MPTCP_EXPECTED_PROGRESS_TARGET  0x219

/*
 * The TCP_INFO socket option is a private API and is subject to change
 */
#pragma pack(4)

#define TCPI_OPT_TIMESTAMPS     0x01
#define TCPI_OPT_SACK           0x02
#define TCPI_OPT_WSCALE         0x04
#define TCPI_OPT_ECN            0x08

#define TCPI_FLAG_LOSSRECOVERY  0x01    /* Currently in loss recovery */
#define TCPI_FLAG_STREAMING_ON  0x02    /* Streaming detection on */

struct tcp_conn_status {
	unsigned int    probe_activated : 1;
	unsigned int    write_probe_failed : 1;
	unsigned int    read_probe_failed : 1;
	unsigned int    conn_probe_failed : 1;
};

/*
 * Add new fields to this structure at the end only. This will preserve
 * binary compatibility.
 */
struct tcp_info {
	u_int8_t        tcpi_state;                     /* TCP FSM state. */
	u_int8_t        tcpi_options;           /* Options enabled on conn. */
	u_int8_t        tcpi_snd_wscale;        /* RFC1323 send shift value. */
	u_int8_t        tcpi_rcv_wscale;        /* RFC1323 recv shift value. */

	u_int32_t       tcpi_flags;                     /* extra flags (TCPI_FLAG_xxx) */

	u_int32_t       tcpi_rto;                       /* Retransmission timeout in milliseconds */
	u_int32_t       tcpi_snd_mss;           /* Max segment size for send. */
	u_int32_t       tcpi_rcv_mss;           /* Max segment size for receive. */

	u_int32_t       tcpi_rttcur;            /* Most recent value of RTT */
	u_int32_t       tcpi_srtt;                      /* Smoothed RTT */
	u_int32_t       tcpi_rttvar;            /* RTT variance */
	u_int32_t       tcpi_rttbest;           /* Best RTT we've seen */

	u_int32_t       tcpi_snd_ssthresh;      /* Slow start threshold. */
	u_int32_t       tcpi_snd_cwnd;          /* Send congestion window. */

	u_int32_t       tcpi_rcv_space;         /* Advertised recv window. */

	u_int32_t       tcpi_snd_wnd;           /* Advertised send window. */
	u_int32_t       tcpi_snd_nxt;           /* Next egress seqno */
	u_int32_t       tcpi_rcv_nxt;           /* Next ingress seqno */

	int32_t         tcpi_last_outif;        /* if_index of interface used to send last */
	u_int32_t       tcpi_snd_sbbytes;       /* bytes in snd buffer including data inflight */

	u_int64_t       tcpi_txpackets __attribute__((aligned(8)));     /* total packets sent */
	u_int64_t       tcpi_txbytes __attribute__((aligned(8)));
	/* total bytes sent */
	u_int64_t       tcpi_txretransmitbytes __attribute__((aligned(8)));
	/* total bytes retransmitted */
	u_int64_t       tcpi_txunacked __attribute__((aligned(8)));
	/* current number of bytes not acknowledged */
	u_int64_t       tcpi_rxpackets __attribute__((aligned(8)));     /* total packets received */
	u_int64_t       tcpi_rxbytes __attribute__((aligned(8)));
	/* total bytes received */
	u_int64_t       tcpi_rxduplicatebytes __attribute__((aligned(8)));
	/* total duplicate bytes received */
	u_int64_t       tcpi_rxoutoforderbytes __attribute__((aligned(8)));
	/* total out of order bytes received */
	u_int64_t       tcpi_snd_bw __attribute__((aligned(8)));        /* measured send bandwidth in bits/sec */
	u_int8_t        tcpi_synrexmits;        /* Number of syn retransmits before connect */
	u_int8_t        tcpi_unused1;
	u_int16_t       tcpi_unused2;
	u_int64_t       tcpi_cell_rxpackets __attribute((aligned(8)));  /* packets received over cellular */
	u_int64_t       tcpi_cell_rxbytes __attribute((aligned(8)));    /* bytes received over cellular */
	u_int64_t       tcpi_cell_txpackets __attribute((aligned(8)));  /* packets transmitted over cellular */
	u_int64_t       tcpi_cell_txbytes __attribute((aligned(8)));    /* bytes transmitted over cellular */
	u_int64_t       tcpi_wifi_rxpackets __attribute((aligned(8)));  /* packets received over Wi-Fi */
	u_int64_t       tcpi_wifi_rxbytes __attribute((aligned(8)));    /* bytes received over Wi-Fi */
	u_int64_t       tcpi_wifi_txpackets __attribute((aligned(8)));  /* packets transmitted over Wi-Fi */
	u_int64_t       tcpi_wifi_txbytes __attribute((aligned(8)));    /* bytes transmitted over Wi-Fi */
	u_int64_t       tcpi_wired_rxpackets __attribute((aligned(8))); /* packets received over Wired */
	u_int64_t       tcpi_wired_rxbytes __attribute((aligned(8)));   /* bytes received over Wired */
	u_int64_t       tcpi_wired_txpackets __attribute((aligned(8))); /* packets transmitted over Wired */
	u_int64_t       tcpi_wired_txbytes __attribute((aligned(8)));   /* bytes transmitted over Wired */
	struct tcp_conn_status  tcpi_connstatus; /* status of connection probes */

	u_int16_t
	    tcpi_tfo_cookie_req:1,     /* Cookie requested? */
	    tcpi_tfo_cookie_rcv:1,     /* Cookie received? */
	    tcpi_tfo_syn_loss:1,       /* Fallback to reg. TCP after SYN-loss */
	    tcpi_tfo_syn_data_sent:1,     /* SYN+data has been sent out */
	    tcpi_tfo_syn_data_acked:1,     /* SYN+data has been fully acknowledged */
	    tcpi_tfo_syn_data_rcv:1,     /* Server received SYN+data with a valid cookie */
	    tcpi_tfo_cookie_req_rcv:1,     /* Server received cookie-request */
	    tcpi_tfo_cookie_sent:1,     /* Server announced cookie */
	    tcpi_tfo_cookie_invalid:1,     /* Server received an invalid cookie */
	    tcpi_tfo_cookie_wrong:1,     /* Our sent cookie was wrong */
	    tcpi_tfo_no_cookie_rcv:1,     /* We did not receive a cookie upon our request */
	    tcpi_tfo_heuristics_disable:1,     /* TFO-heuristics disabled it */
	    tcpi_tfo_send_blackhole:1,     /* A sending-blackhole got detected */
	    tcpi_tfo_recv_blackhole:1,     /* A receiver-blackhole got detected */
	    tcpi_tfo_onebyte_proxy:1;     /* A proxy acknowledges all but one byte of the SYN */

	u_int16_t       tcpi_ecn_client_setup:1,        /* Attempted ECN setup from client side */
	    tcpi_ecn_server_setup:1,                    /* Attempted ECN setup from server side */
	    tcpi_ecn_success:1,                         /* peer negotiated ECN */
	    tcpi_ecn_lost_syn:1,                        /* Lost SYN with ECN setup */
	    tcpi_ecn_lost_synack:1,                     /* Lost SYN-ACK with ECN setup */
	    tcpi_local_peer:1,                          /* Local to the host or the subnet */
	    tcpi_if_cell:1,                     /* Interface is cellular */
	    tcpi_if_wifi:1,                     /* Interface is WiFi */
	    tcpi_if_wired:1,                    /* Interface is wired - ethernet , thunderbolt etc,. */
	    tcpi_if_wifi_infra:1,               /* Interface is wifi infrastructure */
	    tcpi_if_wifi_awdl:1,                /* Interface is wifi AWDL */
	    tcpi_snd_background:1,              /* Using delay based algorithm on sender side */
	    tcpi_rcv_background:1;              /* Using delay based algorithm on receive side */

	u_int32_t       tcpi_ecn_recv_ce;       /* Packets received with CE */
	u_int32_t       tcpi_ecn_recv_cwr;      /* Packets received with CWR */

	u_int32_t       tcpi_rcvoopack;         /* out-of-order packets received */
	u_int32_t       tcpi_pawsdrop;          /* segments dropped due to PAWS */
	u_int32_t       tcpi_sack_recovery_episode; /* SACK recovery episodes */
	u_int32_t       tcpi_reordered_pkts;    /* packets reorderd */
	u_int32_t       tcpi_dsack_sent;        /* Sent DSACK notification */
	u_int32_t       tcpi_dsack_recvd;       /* Received a valid DSACK option */
	u_int32_t       tcpi_flowhash;          /* Unique id for the connection */

	u_int64_t       tcpi_txretransmitpackets __attribute__((aligned(8)));
};

struct tcp_measure_bw_burst {
	u_int32_t       min_burst_size; /* Minimum number of packets to use */
	u_int32_t       max_burst_size; /* Maximum number of packets to use */
};

/*
 * Note that IPv6 link local addresses should have the appropriate scope ID
 */

struct info_tuple {
	u_int8_t        itpl_proto;
	union {
		struct sockaddr         _itpl_sa;
		struct sockaddr_in      _itpl_sin;
		struct sockaddr_in6     _itpl_sin6;
	} itpl_localaddr;
	union {
		struct sockaddr         _itpl_sa;
		struct sockaddr_in      _itpl_sin;
		struct sockaddr_in6     _itpl_sin6;
	} itpl_remoteaddr;
};

#define itpl_local_sa           itpl_localaddr._itpl_sa
#define itpl_local_sin          itpl_localaddr._itpl_sin
#define itpl_local_sin6         itpl_localaddr._itpl_sin6
#define itpl_remote_sa          itpl_remoteaddr._itpl_sa
#define itpl_remote_sin         itpl_remoteaddr._itpl_sin
#define itpl_remote_sin6        itpl_remoteaddr._itpl_sin6

/*
 * TCP connection info auxiliary data (CIAUX_TCP)
 *
 * Do not add new fields to this structure, just add them to tcp_info
 * structure towards the end. This will preserve binary compatibility.
 */
typedef struct conninfo_tcp {
	pid_t                   tcpci_peer_pid; /* loopback peer PID if > 0 */
	struct tcp_info         tcpci_tcp_info; /* TCP info */
} conninfo_tcp_t;

#pragma pack()

struct mptcp_itf_stats {
	uint16_t        ifindex;
	uint16_t        switches;
	uint32_t        is_expensive:1;
	uint64_t        mpis_txbytes __attribute__((aligned(8)));
	uint64_t        mpis_rxbytes __attribute__((aligned(8)));
	uint64_t        mpis_wifi_txbytes __attribute__((aligned(8)));
	uint64_t        mpis_wifi_rxbytes __attribute__((aligned(8)));
	uint64_t        mpis_wired_txbytes __attribute__((aligned(8)));
	uint64_t        mpis_wired_rxbytes __attribute__((aligned(8)));
	uint64_t        mpis_cell_txbytes __attribute__((aligned(8)));
	uint64_t        mpis_cell_rxbytes __attribute__((aligned(8)));
};

/* Version solely used to let libnetcore survive */
#define CONNINFO_MPTCP_VERSION  3
typedef struct conninfo_multipathtcp {
	uint32_t        mptcpci_subflow_count;
	uint32_t        mptcpci_switch_count;
	sae_connid_t    mptcpci_subflow_connids[4];

	uint64_t        mptcpci_init_rxbytes;
	uint64_t        mptcpci_init_txbytes;

#define MPTCP_ITFSTATS_SIZE     4
	struct mptcp_itf_stats mptcpci_itfstats[MPTCP_ITFSTATS_SIZE];

	uint32_t        mptcpci_flags;
#define MPTCPCI_FIRSTPARTY      0x01
} conninfo_multipathtcp_t;

#endif /* PRIVATE */

struct tcp_connection_info {
	u_int8_t        tcpi_state;     /* connection state */
	u_int8_t        tcpi_snd_wscale; /* Window scale for send window */
	u_int8_t        tcpi_rcv_wscale; /* Window scale for receive window */
	u_int8_t        __pad1;
	u_int32_t       tcpi_options;   /* TCP options supported */
#define TCPCI_OPT_TIMESTAMPS    0x00000001 /* Timestamps enabled */
#define TCPCI_OPT_SACK          0x00000002 /* SACK enabled */
#define TCPCI_OPT_WSCALE        0x00000004 /* Window scaling enabled */
#define TCPCI_OPT_ECN           0x00000008 /* ECN enabled */
	u_int32_t       tcpi_flags;     /* flags */
#define TCPCI_FLAG_LOSSRECOVERY 0x00000001
#define TCPCI_FLAG_REORDERING_DETECTED  0x00000002
	u_int32_t       tcpi_rto;       /* retransmit timeout in ms */
	u_int32_t       tcpi_maxseg;    /* maximum segment size supported */
	u_int32_t       tcpi_snd_ssthresh; /* slow start threshold in bytes */
	u_int32_t       tcpi_snd_cwnd;  /* send congestion window in bytes */
	u_int32_t       tcpi_snd_wnd;   /* send widnow in bytes */
	u_int32_t       tcpi_snd_sbbytes; /* bytes in send socket buffer, including in-flight data */
	u_int32_t       tcpi_rcv_wnd;   /* receive window in bytes*/
	u_int32_t       tcpi_rttcur;    /* most recent RTT in ms */
	u_int32_t       tcpi_srtt;      /* average RTT in ms */
	u_int32_t       tcpi_rttvar;    /* RTT variance */
	u_int32_t
	    tcpi_tfo_cookie_req:1,             /* Cookie requested? */
	    tcpi_tfo_cookie_rcv:1,             /* Cookie received? */
	    tcpi_tfo_syn_loss:1,               /* Fallback to reg. TCP after SYN-loss */
	    tcpi_tfo_syn_data_sent:1,             /* SYN+data has been sent out */
	    tcpi_tfo_syn_data_acked:1,             /* SYN+data has been fully acknowledged */
	    tcpi_tfo_syn_data_rcv:1,             /* Server received SYN+data with a valid cookie */
	    tcpi_tfo_cookie_req_rcv:1,             /* Server received cookie-request */
	    tcpi_tfo_cookie_sent:1,             /* Server announced cookie */
	    tcpi_tfo_cookie_invalid:1,             /* Server received an invalid cookie */
	    tcpi_tfo_cookie_wrong:1,             /* Our sent cookie was wrong */
	    tcpi_tfo_no_cookie_rcv:1,             /* We did not receive a cookie upon our request */
	    tcpi_tfo_heuristics_disable:1,             /* TFO-heuristics disabled it */
	    tcpi_tfo_send_blackhole:1,             /* A sending-blackhole got detected */
	    tcpi_tfo_recv_blackhole:1,             /* A receiver-blackhole got detected */
	    tcpi_tfo_onebyte_proxy:1,             /* A proxy acknowledges all but one byte of the SYN */
	    __pad2:17;
	u_int64_t       tcpi_txpackets __attribute__((aligned(8)));
	u_int64_t       tcpi_txbytes __attribute__((aligned(8)));
	u_int64_t       tcpi_txretransmitbytes __attribute__((aligned(8)));
	u_int64_t       tcpi_rxpackets __attribute__((aligned(8)));
	u_int64_t       tcpi_rxbytes __attribute__((aligned(8)));
	u_int64_t       tcpi_rxoutoforderbytes __attribute__((aligned(8)));
	u_int64_t       tcpi_txretransmitpackets __attribute__((aligned(8)));
};
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

#endif
