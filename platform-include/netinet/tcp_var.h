/*
 * Copyright (c) 2000-2014 Apple Inc. All rights reserved.
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
 * Copyright (c) 1982, 1986, 1993, 1994, 1995
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
 *	@(#)tcp_var.h	8.4 (Berkeley) 5/24/95
 * $FreeBSD: src/sys/netinet/tcp_var.h,v 1.56.2.8 2001/08/22 00:59:13 silby Exp $
 */

#ifndef _NETINET_TCP_VAR_H_
#define _NETINET_TCP_VAR_H_
#include <sys/appleapiopts.h>
#include <sys/queue.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp_timer.h>

#if defined(__LP64__)
#define _TCPCB_PTR(x)			u_int32_t
#define _TCPCB_LIST_HEAD(name, type)	\
struct name {				\
	u_int32_t	lh_first;	\
};
#else
#define _TCPCB_PTR(x)			x
#define _TCPCB_LIST_HEAD(name, type)	LIST_HEAD(name, type)
#endif

struct tseg_qent;
_TCPCB_LIST_HEAD(tsegqe_head, tseg_qent);

struct tcpcb {
	struct	tsegqe_head t_segq;
	int	t_dupacks;		/* consecutive dup acks recd */
	u_int32_t unused;		/* unused now: was t_template */

	int	t_timer[TCPT_NTIMERS_EXT];	/* tcp timers */

	_TCPCB_PTR(struct inpcb *) t_inpcb;	/* back pointer to internet pcb */
	int	t_state;		/* state of this connection */
	u_int	t_flags;
#define	TF_ACKNOW	0x00001		/* ack peer immediately */
#define	TF_DELACK	0x00002		/* ack, but try to delay it */
#define	TF_NODELAY	0x00004		/* don't delay packets to coalesce */
#define	TF_NOOPT	0x00008		/* don't use tcp options */
#define	TF_SENTFIN	0x00010		/* have sent FIN */
#define	TF_REQ_SCALE	0x00020		/* have/will request window scaling */
#define	TF_RCVD_SCALE	0x00040		/* other side has requested scaling */
#define	TF_REQ_TSTMP	0x00080		/* have/will request timestamps */
#define	TF_RCVD_TSTMP	0x00100		/* a timestamp was received in SYN */
#define	TF_SACK_PERMIT	0x00200		/* other side said I could SACK */
#define	TF_NEEDSYN	0x00400		/* send SYN (implicit state) */
#define	TF_NEEDFIN	0x00800		/* send FIN (implicit state) */
#define	TF_NOPUSH	0x01000		/* don't push */
#define	TF_REQ_CC	0x02000		/* have/will request CC */
#define	TF_RCVD_CC	0x04000		/* a CC was received in SYN */
#define	TF_SENDCCNEW	0x08000		/* send CCnew instead of CC in SYN */
#define	TF_MORETOCOME	0x10000		/* More data to be appended to sock */
#define	TF_LQ_OVERFLOW	0x20000		/* listen queue overflow */
#define	TF_RXWIN0SENT	0x40000		/* sent a receiver win 0 in response */
#define	TF_SLOWLINK	0x80000		/* route is a on a modem speed link */

	int	t_force;		/* 1 if forcing out a byte */

	tcp_seq	snd_una;		/* send unacknowledged */
	tcp_seq	snd_max;		/* highest sequence number sent;
					 * used to recognize retransmits
					 */
	tcp_seq	snd_nxt;		/* send next */
	tcp_seq	snd_up;			/* send urgent pointer */

	tcp_seq	snd_wl1;		/* window update seg seq number */
	tcp_seq	snd_wl2;		/* window update seg ack number */
	tcp_seq	iss;			/* initial send sequence number */
	tcp_seq	irs;			/* initial receive sequence number */

	tcp_seq	rcv_nxt;		/* receive next */
	tcp_seq	rcv_adv;		/* advertised window */
	u_int32_t rcv_wnd;		/* receive window */
	tcp_seq	rcv_up;			/* receive urgent pointer */

	u_int32_t snd_wnd;		/* send window */
	u_int32_t snd_cwnd;		/* congestion-controlled window */
	u_int32_t snd_ssthresh;		/* snd_cwnd size threshold for
					 * for slow start exponential to
					 * linear switch
					 */
	u_int	t_maxopd;		/* mss plus options */

	u_int32_t t_rcvtime;		/* time at which a packet was received */
	u_int32_t t_starttime;		/* time connection was established */
	int	t_rtttime;		/* round trip time */
	tcp_seq	t_rtseq;		/* sequence number being timed */

	int	t_rxtcur;		/* current retransmit value (ticks) */
	u_int	t_maxseg;		/* maximum segment size */
	int	t_srtt;			/* smoothed round-trip time */
	int	t_rttvar;		/* variance in round-trip time */

	int	t_rxtshift;		/* log(2) of rexmt exp. backoff */
	u_int	t_rttmin;		/* minimum rtt allowed */
	u_int32_t t_rttupdated;		/* number of times rtt sampled */
	u_int32_t max_sndwnd;		/* largest window peer has offered */

	int	t_softerror;		/* possible error not yet reported */
/* out-of-band data */
	char	t_oobflags;		/* have some */
	char	t_iobc;			/* input character */
#define	TCPOOB_HAVEDATA	0x01
#define	TCPOOB_HADDATA	0x02
/* RFC 1323 variables */
	u_char	snd_scale;		/* window scaling for send window */
	u_char	rcv_scale;		/* window scaling for recv window */
	u_char	request_r_scale;	/* pending window scaling */
	u_char	requested_s_scale;
	u_int32_t ts_recent;		/* timestamp echo data */

	u_int32_t ts_recent_age;	/* when last updated */
	tcp_seq	last_ack_sent;
/* RFC 1644 variables */
	tcp_cc	cc_send;		/* send connection count */
	tcp_cc	cc_recv;		/* receive connection count */
	tcp_seq	snd_recover;		/* for use in fast recovery */
/* experimental */
	u_int32_t snd_cwnd_prev;	/* cwnd prior to retransmit */
	u_int32_t snd_ssthresh_prev;	/* ssthresh prior to retransmit */
	u_int32_t t_badrxtwin;		/* window for retransmit recovery */
};


/*
 * TCP statistics.
 * Many of these should be kept per connection,
 * but that's inconvenient at the moment.
 */
struct	tcpstat {
	u_int32_t	tcps_connattempt;	/* connections initiated */
	u_int32_t	tcps_accepts;		/* connections accepted */
	u_int32_t	tcps_connects;		/* connections established */
	u_int32_t	tcps_drops;		/* connections dropped */
	u_int32_t	tcps_conndrops;		/* embryonic connections dropped */
	u_int32_t	tcps_closed;		/* conn. closed (includes drops) */
	u_int32_t	tcps_segstimed;		/* segs where we tried to get rtt */
	u_int32_t	tcps_rttupdated;	/* times we succeeded */
	u_int32_t	tcps_delack;		/* delayed acks sent */
	u_int32_t	tcps_timeoutdrop;	/* conn. dropped in rxmt timeout */
	u_int32_t	tcps_rexmttimeo;	/* retransmit timeouts */
	u_int32_t	tcps_persisttimeo;	/* persist timeouts */
	u_int32_t	tcps_keeptimeo;		/* keepalive timeouts */
	u_int32_t	tcps_keepprobe;		/* keepalive probes sent */
	u_int32_t	tcps_keepdrops;		/* connections dropped in keepalive */

	u_int32_t	tcps_sndtotal;		/* total packets sent */
	u_int32_t	tcps_sndpack;		/* data packets sent */
	u_int32_t	tcps_sndbyte;		/* data bytes sent */
	u_int32_t	tcps_sndrexmitpack;	/* data packets retransmitted */
	u_int32_t	tcps_sndrexmitbyte;	/* data bytes retransmitted */
	u_int32_t	tcps_sndacks;		/* ack-only packets sent */
	u_int32_t	tcps_sndprobe;		/* window probes sent */
	u_int32_t	tcps_sndurg;		/* packets sent with URG only */
	u_int32_t	tcps_sndwinup;		/* window update-only packets sent */
	u_int32_t	tcps_sndctrl;		/* control (SYN|FIN|RST) packets sent */

	u_int32_t	tcps_rcvtotal;		/* total packets received */
	u_int32_t	tcps_rcvpack;		/* packets received in sequence */
	u_int32_t	tcps_rcvbyte;		/* bytes received in sequence */
	u_int32_t	tcps_rcvbadsum;		/* packets received with ccksum errs */
	u_int32_t	tcps_rcvbadoff;		/* packets received with bad offset */
	u_int32_t	tcps_rcvmemdrop;	/* packets dropped for lack of memory */
	u_int32_t	tcps_rcvshort;		/* packets received too short */
	u_int32_t	tcps_rcvduppack;	/* duplicate-only packets received */
	u_int32_t	tcps_rcvdupbyte;	/* duplicate-only bytes received */
	u_int32_t	tcps_rcvpartduppack;	/* packets with some duplicate data */
	u_int32_t	tcps_rcvpartdupbyte;	/* dup. bytes in part-dup. packets */
	u_int32_t	tcps_rcvoopack;		/* out-of-order packets received */
	u_int32_t	tcps_rcvoobyte;		/* out-of-order bytes received */
	u_int32_t	tcps_rcvpackafterwin;	/* packets with data after window */
	u_int32_t	tcps_rcvbyteafterwin;	/* bytes rcvd after window */
	u_int32_t	tcps_rcvafterclose;	/* packets rcvd after "close" */
	u_int32_t	tcps_rcvwinprobe;	/* rcvd window probe packets */
	u_int32_t	tcps_rcvdupack;		/* rcvd duplicate acks */
	u_int32_t	tcps_rcvacktoomuch;	/* rcvd acks for unsent data */
	u_int32_t	tcps_rcvackpack;	/* rcvd ack packets */
	u_int32_t	tcps_rcvackbyte;	/* bytes acked by rcvd acks */
	u_int32_t	tcps_rcvwinupd;		/* rcvd window update packets */
	u_int32_t	tcps_pawsdrop;		/* segments dropped due to PAWS */
	u_int32_t	tcps_predack;		/* times hdr predict ok for acks */
	u_int32_t	tcps_preddat;		/* times hdr predict ok for data pkts */
	u_int32_t	tcps_pcbcachemiss;
	u_int32_t	tcps_cachedrtt;		/* times cached RTT in route updated */
	u_int32_t	tcps_cachedrttvar;	/* times cached rttvar updated */
	u_int32_t	tcps_cachedssthresh;	/* times cached ssthresh updated */
	u_int32_t	tcps_usedrtt;		/* times RTT initialized from route */
	u_int32_t	tcps_usedrttvar;	/* times RTTVAR initialized from rt */
	u_int32_t	tcps_usedssthresh;	/* times ssthresh initialized from rt*/
	u_int32_t	tcps_persistdrop;	/* timeout in persist state */
	u_int32_t	tcps_badsyn;		/* bogus SYN, e.g. premature ACK */
	u_int32_t	tcps_mturesent;		/* resends due to MTU discovery */
	u_int32_t	tcps_listendrop;	/* listen queue overflows */

	/* new stats from FreeBSD 5.4 sync up */
	u_int32_t	tcps_minmssdrops;	/* average minmss too low drops */
	u_int32_t	tcps_sndrexmitbad;	/* unnecessary packet retransmissions */
	u_int32_t	tcps_badrst;		/* ignored RSTs in the window */

	u_int32_t	tcps_sc_added;		/* entry added to syncache */
	u_int32_t	tcps_sc_retransmitted;	/* syncache entry was retransmitted */
	u_int32_t	tcps_sc_dupsyn;		/* duplicate SYN packet */
	u_int32_t	tcps_sc_dropped;	/* could not reply to packet */
	u_int32_t	tcps_sc_completed;	/* successful extraction of entry */
	u_int32_t	tcps_sc_bucketoverflow;	/* syncache per-bucket limit hit */
	u_int32_t	tcps_sc_cacheoverflow;	/* syncache cache limit hit */
	u_int32_t	tcps_sc_reset;		/* RST removed entry from syncache */
	u_int32_t	tcps_sc_stale;		/* timed out or listen socket gone */
	u_int32_t	tcps_sc_aborted;	/* syncache entry aborted */
	u_int32_t	tcps_sc_badack;		/* removed due to bad ACK */
	u_int32_t	tcps_sc_unreach;	/* ICMP unreachable received */
	u_int32_t	tcps_sc_zonefail;	/* zalloc() failed */
	u_int32_t	tcps_sc_sendcookie;	/* SYN cookie sent */
	u_int32_t	tcps_sc_recvcookie;	/* SYN cookie received */

	u_int32_t	tcps_hc_added;		/* entry added to hostcache */
	u_int32_t	tcps_hc_bucketoverflow;	/* hostcache per bucket limit hit */

	/* SACK related stats */
	u_int32_t	tcps_sack_recovery_episode; /* SACK recovery episodes */
	u_int32_t 	tcps_sack_rexmits;	    /* SACK rexmit segments   */
	u_int32_t 	tcps_sack_rexmit_bytes;	    /* SACK rexmit bytes      */
	u_int32_t 	tcps_sack_rcv_blocks;	    /* SACK blocks (options) received */
	u_int32_t 	tcps_sack_send_blocks;	    /* SACK blocks (options) sent     */
	u_int32_t 	tcps_sack_sboverflow;	    /* SACK sendblock overflow   */

	u_int32_t	tcps_bg_rcvtotal;	/* total background packets received */
	u_int32_t	tcps_rxtfindrop;	/* drop conn after retransmitting FIN */
	u_int32_t	tcps_fcholdpacket;	/* packets withheld because of flow control */

	/* LRO related stats */
	u_int32_t	tcps_coalesced_pack;	/* number of coalesced packets */
	u_int32_t	tcps_flowtbl_full;	/* times flow table was full */
	u_int32_t	tcps_flowtbl_collision;	/* collisions in flow tbl */
	u_int32_t	tcps_lro_twopack;	/* 2 packets coalesced */
	u_int32_t	tcps_lro_multpack;	/* 3 or 4 pkts coalesced */
	u_int32_t	tcps_lro_largepack;	/* 5 or more pkts coalesced */

	u_int32_t	tcps_limited_txt;	/* Limited transmit used */
	u_int32_t	tcps_early_rexmt;	/* Early retransmit used */
	u_int32_t	tcps_sack_ackadv;	/* Cumulative ack advanced along with sack */

	/* Checksum related stats */
	u_int32_t	tcps_rcv_swcsum;	/* tcp swcksum (inbound), packets */
	u_int32_t	tcps_rcv_swcsum_bytes;	/* tcp swcksum (inbound), bytes */
	u_int32_t	tcps_rcv6_swcsum;	/* tcp6 swcksum (inbound), packets */
	u_int32_t	tcps_rcv6_swcsum_bytes;	/* tcp6 swcksum (inbound), bytes */
	u_int32_t	tcps_snd_swcsum;	/* tcp swcksum (outbound), packets */
	u_int32_t	tcps_snd_swcsum_bytes;	/* tcp swcksum (outbound), bytes */
	u_int32_t	tcps_snd6_swcsum;	/* tcp6 swcksum (outbound), packets */
	u_int32_t	tcps_snd6_swcsum_bytes;	/* tcp6 swcksum (outbound), bytes */
	u_int32_t	tcps_msg_unopkts;	/* unordered packet on TCP msg stream */
	u_int32_t	tcps_msg_unoappendfail; /* failed to append unordered pkt */
	u_int32_t	tcps_msg_sndwaithipri; /* send is waiting for high priority data */

	/* MPTCP Related stats */
	u_int32_t	tcps_invalid_mpcap;	/* Invalid MPTCP capable opts */
	u_int32_t	tcps_invalid_joins;	/* Invalid MPTCP joins */
	u_int32_t	tcps_mpcap_fallback;	/* TCP fallback in primary */
	u_int32_t	tcps_join_fallback;	/* No MPTCP in secondary */
	u_int32_t	tcps_estab_fallback;	/* DSS option dropped */
	u_int32_t	tcps_invalid_opt;	/* Catchall error stat */
	u_int32_t	tcps_mp_outofwin;	/* Packet lies outside the
						   shared recv window */
	u_int32_t	tcps_mp_reducedwin;	/* Reduced subflow window */
	u_int32_t	tcps_mp_badcsum;	/* Bad DSS csum */
	u_int32_t	tcps_mp_oodata;		/* Out of order data */
	u_int32_t	tcps_mp_switches;	/* number of subflow switch */
	u_int32_t	tcps_mp_rcvtotal;	/* number of rcvd packets */
	u_int32_t	tcps_mp_rcvbytes;	/* number of bytes received */
	u_int32_t	tcps_mp_sndpacks;	/* number of data packs sent */
	u_int32_t	tcps_mp_sndbytes;	/* number of bytes sent */
	u_int32_t	tcps_join_rxmts;	/* join ack retransmits */
	u_int32_t	tcps_tailloss_rto;	/* RTO due to tail loss */
	u_int32_t	tcps_reordered_pkts;	/* packets reorderd */
	u_int32_t	tcps_recovered_pkts;	/* recovered after loss */
	u_int32_t	tcps_pto;		/* probe timeout */
	u_int32_t	tcps_rto_after_pto;	/* RTO after a probe */
	u_int32_t	tcps_tlp_recovery;	/* TLP induced fast recovery */
	u_int32_t	tcps_tlp_recoverlastpkt; /* TLP recoverd last pkt */
	u_int32_t	tcps_ecn_setup;		/* connection negotiated ECN */
	u_int32_t	tcps_sent_cwr;		/* Sent CWR, ECE received */
	u_int32_t	tcps_sent_ece;		/* Sent ECE notification */
	u_int32_t	tcps_detect_reordering; /* Detect pkt reordering */
	u_int32_t	tcps_delay_recovery;	/* Delay fast recovery */
	u_int32_t	tcps_avoid_rxmt;	/* Retransmission was avoided */
	u_int32_t	tcps_unnecessary_rxmt;	/* Retransmission was not needed */
	u_int32_t	tcps_nostretchack;	/* disabled stretch ack algorithm on a connection */
	u_int32_t	tcps_rescue_rxmt;	/* SACK rescue retransmit */
	u_int32_t	tcps_pto_in_recovery;	/* PTO during fast recovery */
	u_int32_t	tcps_pmtudbh_reverted;	/* PMTU Blackhole detection, segement size reverted */
};

struct tcpstat_local {
	u_int64_t badformat;
	u_int64_t unspecv6;
	u_int64_t synfin;
	u_int64_t badformatipsec;
	u_int64_t noconnnolist;
	u_int64_t noconnlist;
	u_int64_t listbadsyn;
	u_int64_t icmp6unreach;
	u_int64_t deprecate6;
	u_int64_t ooopacket;
	u_int64_t rstinsynrcv;
	u_int64_t dospacket;
	u_int64_t cleanup;
	u_int64_t synwindow;
};

#pragma pack(4)

/*
 * TCB structure exported to user-land via sysctl(3).
 * Evil hack: declare only if in_pcb.h and sys/socketvar.h have been
 * included.  Not all of our clients do.
 */

struct  xtcpcb {
        u_int32_t       xt_len;
        struct  inpcb   xt_inp;
        struct  tcpcb   xt_tp;
        struct  xsocket xt_socket;
        u_quad_t        xt_alignment_hack;
};

#if !CONFIG_EMBEDDED

struct  xtcpcb64 {
        u_int32_t      		xt_len;
        struct xinpcb64		xt_inpcb;

        u_int64_t t_segq;
        int     t_dupacks;              /* consecutive dup acks recd */

        int t_timer[TCPT_NTIMERS_EXT];  /* tcp timers */

        int     t_state;                /* state of this connection */
        u_int   t_flags;

        int     t_force;                /* 1 if forcing out a byte */

        tcp_seq snd_una;                /* send unacknowledged */
        tcp_seq snd_max;                /* highest sequence number sent;
                                         * used to recognize retransmits
                                         */
        tcp_seq snd_nxt;                /* send next */
        tcp_seq snd_up;                 /* send urgent pointer */

        tcp_seq snd_wl1;                /* window update seg seq number */
        tcp_seq snd_wl2;                /* window update seg ack number */
        tcp_seq iss;                    /* initial send sequence number */
        tcp_seq irs;                    /* initial receive sequence number */

        tcp_seq rcv_nxt;                /* receive next */
        tcp_seq rcv_adv;                /* advertised window */
        u_int32_t rcv_wnd;              /* receive window */
        tcp_seq rcv_up;                 /* receive urgent pointer */

        u_int32_t snd_wnd;              /* send window */
        u_int32_t snd_cwnd;             /* congestion-controlled window */
        u_int32_t snd_ssthresh;         /* snd_cwnd size threshold for
                                         * for slow start exponential to
                                         * linear switch
                                         */
        u_int   t_maxopd;               /* mss plus options */

        u_int32_t t_rcvtime;            /* time at which a packet was received */
        u_int32_t t_starttime;          /* time connection was established */
        int     t_rtttime;              /* round trip time */
        tcp_seq t_rtseq;                /* sequence number being timed */

        int     t_rxtcur;               /* current retransmit value (ticks) */
        u_int   t_maxseg;               /* maximum segment size */
        int     t_srtt;                 /* smoothed round-trip time */
        int     t_rttvar;               /* variance in round-trip time */

        int     t_rxtshift;             /* log(2) of rexmt exp. backoff */
        u_int   t_rttmin;               /* minimum rtt allowed */
        u_int32_t t_rttupdated;         /* number of times rtt sampled */
        u_int32_t max_sndwnd;           /* largest window peer has offered */

        int     t_softerror;            /* possible error not yet reported */
/* out-of-band data */
        char    t_oobflags;             /* have some */
        char    t_iobc;                 /* input character */
/* RFC 1323 variables */
        u_char  snd_scale;              /* window scaling for send window */
        u_char  rcv_scale;              /* window scaling for recv window */
        u_char  request_r_scale;        /* pending window scaling */
        u_char  requested_s_scale;
        u_int32_t ts_recent;            /* timestamp echo data */

        u_int32_t ts_recent_age;        /* when last updated */
        tcp_seq last_ack_sent;
/* RFC 1644 variables */
        tcp_cc  cc_send;                /* send connection count */
        tcp_cc  cc_recv;                /* receive connection count */
        tcp_seq snd_recover;            /* for use in fast recovery */
/* experimental */
        u_int32_t snd_cwnd_prev;        /* cwnd prior to retransmit */
        u_int32_t snd_ssthresh_prev;    /* ssthresh prior to retransmit */
        u_int32_t t_badrxtwin;          /* window for retransmit recovery */

        u_quad_t		xt_alignment_hack;
};

#endif /* !CONFIG_EMBEDDED */


#pragma pack()

/*
 * Names for TCP sysctl objects
 */
#define	TCPCTL_DO_RFC1323	1	/* use RFC-1323 extensions */
#define	TCPCTL_DO_RFC1644	2	/* use RFC-1644 extensions */
#define	TCPCTL_MSSDFLT		3	/* MSS default */
#define TCPCTL_STATS		4	/* statistics (read-only) */
#define	TCPCTL_RTTDFLT		5	/* default RTT estimate */
#define	TCPCTL_KEEPIDLE		6	/* keepalive idle timer */
#define	TCPCTL_KEEPINTVL	7	/* interval to send keepalives */
#define	TCPCTL_SENDSPACE	8	/* send buffer space */
#define	TCPCTL_RECVSPACE	9	/* receive buffer space */
#define	TCPCTL_KEEPINIT		10	/* timeout for establishing syn */
#define	TCPCTL_PCBLIST		11	/* list of all outstanding PCBs */
#define	TCPCTL_DELACKTIME	12	/* time before sending delayed ACK */
#define	TCPCTL_V6MSSDFLT	13	/* MSS default for IPv6 */
#define	TCPCTL_MAXID		14


#endif /* _NETINET_TCP_VAR_H_ */
