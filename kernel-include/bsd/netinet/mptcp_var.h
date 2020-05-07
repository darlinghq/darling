/*
 * Copyright (c) 2012-2017 Apple Inc. All rights reserved.
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

#ifndef _NETINET_MPTCP_VAR_H_
#define _NETINET_MPTCP_VAR_H_

#ifdef PRIVATE
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#ifdef BSD_KERNEL_PRIVATE
#include <sys/queue.h>
#include <sys/protosw.h>
#include <kern/locks.h>
#include <mach/boolean.h>
#include <netinet/mp_pcb.h>
#include <netinet/tcp_var.h>
#include <os/log.h>

struct mpt_itf_info {
	uint32_t ifindex;
	uint32_t has_v4_conn:1,
	    has_v6_conn:1,
	    has_nat64_conn:1,
	    no_mptcp_support:1;
};

/*
 * MPTCP Session
 *
 * This is an extension to the multipath PCB specific for MPTCP, protected by
 * the per-PCB mpp_lock (also the socket's lock);
 */
struct mptses {
	struct mppcb    *mpte_mppcb;            /* back ptr to multipath PCB */
	struct mptcb    *mpte_mptcb;            /* ptr to MPTCP PCB */
	TAILQ_HEAD(, mptopt) mpte_sopts;        /* list of socket options */
	TAILQ_HEAD(, mptsub) mpte_subflows;     /* list of subflows */
	uint16_t        mpte_numflows;          /* # of subflows in list */
	uint16_t        mpte_nummpcapflows;     /* # of MP_CAP subflows */
	sae_associd_t   mpte_associd;           /* MPTCP association ID */
	sae_connid_t    mpte_connid_last;       /* last used connection ID */

	uint64_t        mpte_time_target;
	thread_call_t   mpte_time_thread;

	uint32_t        mpte_last_cellicon_set;
	uint32_t        mpte_cellicon_increments;

	union {
		/* Source address of initial subflow */
		struct sockaddr _mpte_src;
		struct sockaddr_in _mpte_src_v4;
		struct sockaddr_in6 _mpte_src_v6;
	} mpte_u_src;
#define mpte_src mpte_u_src._mpte_src
#define __mpte_src_v4 mpte_u_src._mpte_src_v4
#define __mpte_src_v6 mpte_u_src._mpte_src_v6
	union {
		/* Destination address of initial subflow */
		struct sockaddr _mpte_dst;
		struct sockaddr_in _mpte_dst_v4;
		struct sockaddr_in6 _mpte_dst_v6;
	} mpte_u_dst;
#define mpte_dst mpte_u_dst._mpte_dst
#define __mpte_dst_v4 mpte_u_dst._mpte_dst_v4
#define __mpte_dst_v6 mpte_u_dst._mpte_dst_v6

	struct sockaddr_in mpte_dst_v4_nat64;

	struct sockaddr_in mpte_dst_unicast_v4;
	struct sockaddr_in6 mpte_dst_unicast_v6;

	uint16_t        mpte_alternate_port;    /* Alternate port for subflow establishment (network-byte-order) */

	int mpte_epid;
	uuid_t mpte_euuid;

	struct mptsub   *mpte_active_sub;       /* ptr to last active subf */
	uint16_t mpte_flags;                    /* per mptcp session flags */
#define MPTE_SND_REM_ADDR       0x01            /* Send Remove_addr option */
#define MPTE_SVCTYPE_CHECKED    0x02            /* Did entitlement-check for service-type */
#define MPTE_FIRSTPARTY         0x04            /* First-party app used multipath_extended entitlement */
#define MPTE_ACCESS_GRANTED     0x08            /* Access to cellular has been granted for this connection */
#define MPTE_FORCE_ENABLE       0x10            /* For MPTCP regardless of heuristics to detect middleboxes */
#define MPTE_IN_WORKLOOP        0x20            /* Are we currently inside the workloop ? */
#define MPTE_WORKLOOP_RELAUNCH  0x40            /* Another event got queued, we should restart the workloop */
#define MPTE_UNICAST_IP         0x80            /* New subflows are only being established towards the unicast IP in the ADD_ADDR */
#define MPTE_CELL_PROHIBITED    0x100           /* Cell access has been prohibited based on signal quality */
	uint8_t mpte_svctype;                   /* MPTCP Service type */
	uint8_t mpte_lost_aid;                  /* storing lost address id */
	uint8_t mpte_addrid_last;               /* storing address id parm */

#define MPTE_ITFINFO_SIZE       4
	uint32_t        mpte_itfinfo_size;
	struct mpt_itf_info     _mpte_itfinfo[MPTE_ITFINFO_SIZE];
	struct mpt_itf_info     *mpte_itfinfo;

	struct mbuf             *mpte_reinjectq;

	/* The below is used for stats */
	uint32_t        mpte_subflow_switches;  /* Number of subflow-switches in sending */
	uint32_t        mpte_used_cell:1,
	    mpte_used_wifi:1,
	    mpte_initial_cell:1,
	    mpte_triggered_cell,
	    mpte_handshake_success:1;

	struct mptcp_itf_stats  mpte_itfstats[MPTCP_ITFSTATS_SIZE];
	uint64_t                mpte_init_txbytes __attribute__((aligned(8)));
	uint64_t                mpte_init_rxbytes __attribute__((aligned(8)));
};

static inline struct socket *
mptetoso(struct mptses *mpte)
{
	return mpte->mpte_mppcb->mpp_socket;
}

static inline struct mptses *
mptompte(struct mppcb *mp)
{
	return (struct mptses *)mp->mpp_pcbe;
}

static inline struct mptses *
mpsotompte(struct socket *so)
{
	return mptompte(mpsotomppcb(so));
}

static inline boolean_t
mpp_try_lock(struct mppcb *mp)
{
	if (!lck_mtx_try_lock(&mp->mpp_lock)) {
		return false;
	}

	VERIFY(!(mp->mpp_flags & MPP_INSIDE_OUTPUT));
	VERIFY(!(mp->mpp_flags & MPP_INSIDE_INPUT));

	return true;
}

static inline void
mpp_lock(struct mppcb *mp)
{
	lck_mtx_lock(&mp->mpp_lock);
	VERIFY(!(mp->mpp_flags & MPP_INSIDE_OUTPUT));
	VERIFY(!(mp->mpp_flags & MPP_INSIDE_INPUT));
}

static inline void
mpp_unlock(struct mppcb *mp)
{
	VERIFY(!(mp->mpp_flags & MPP_INSIDE_OUTPUT));
	VERIFY(!(mp->mpp_flags & MPP_INSIDE_INPUT));
	lck_mtx_unlock(&mp->mpp_lock);
}

static inline lck_mtx_t *
mpp_getlock(struct mppcb *mp, int flags)
{
	if (flags & PR_F_WILLUNLOCK) {
		VERIFY(!(mp->mpp_flags & MPP_INSIDE_OUTPUT));
		VERIFY(!(mp->mpp_flags & MPP_INSIDE_INPUT));
	}

	return &mp->mpp_lock;
}

static inline int
mptcp_subflow_cwnd_space(struct socket *so)
{
	struct tcpcb *tp = sototcpcb(so);
	int cwnd = min(tp->snd_wnd, tp->snd_cwnd) - (so->so_snd.sb_cc);

	return min(cwnd, sbspace(&so->so_snd));
}


/*
 * MPTCP socket options
 */
struct mptopt {
	TAILQ_ENTRY(mptopt)     mpo_entry;      /* glue to other options */
	uint32_t                mpo_flags;      /* see flags below */
	int                     mpo_level;      /* sopt_level */
	int                     mpo_name;       /* sopt_name */
	int                     mpo_intval;     /* sopt_val */
};

#define MPOF_ATTACHED           0x1     /* attached to MP socket */
#define MPOF_SUBFLOW_OK         0x2     /* can be issued on subflow socket */
#define MPOF_INTERIM            0x4     /* has not been issued on any subflow */

/*
 * MPTCP subflow
 *
 * Note that mpts_flags and mpts_evctl are modified via atomic operations.
 */
struct mptsub {
	TAILQ_ENTRY(mptsub)     mpts_entry;     /* glue to peer subflows */
	uint32_t                mpts_refcnt;    /* reference count */
	uint32_t                mpts_flags;     /* see flags below */
	uint32_t                mpts_evctl;     /* subflow control events */
	sae_connid_t            mpts_connid;    /* subflow connection ID */
	int                     mpts_oldintval; /* sopt_val before sosetopt  */
	struct mptses           *mpts_mpte;     /* back ptr to MPTCP session */
	struct socket           *mpts_socket;   /* subflow socket */
	struct sockaddr         *mpts_src;      /* source address */

	union {
		/* destination address */
		struct sockaddr         _mpts_dst;
		struct sockaddr_in      _mpts_dst_v4;
		struct sockaddr_in6     _mpts_dst_v6;
	} mpts_u_dst;
#define mpts_dst mpts_u_dst._mpts_dst
#define __mpts_dst_v4 mpts_u_dst._mpts_dst_v4
#define __mpts_dst_v6 mpts_u_dst._mpts_dst_v6
	u_int32_t               mpts_rel_seq;   /* running count of subflow # */
	u_int32_t               mpts_iss;       /* Initial sequence number, taking TFO into account */
	u_int32_t               mpts_ifscope;   /* scoped to the interface */
	uint32_t                mpts_probesoon; /* send probe after probeto */
	uint32_t                mpts_probecnt;  /* number of probes sent */
	uint32_t                mpts_maxseg;    /* cached value of t_maxseg */
};

/*
 * Valid values for mpts_flags.  In particular:
 *
 *    - MP_CAPABLE means that the connection is successfully established as
 *	MPTCP and data transfer may occur, but is not yet ready for multipath-
 *	related semantics until MP_READY.  I.e. if this is on the first subflow,
 *	it causes the MPTCP socket to transition to a connected state, except
 *	that additional subflows will not be established; they will be marked
 *	with PENDING and will be processed when the first subflow is marked
 *	with MP_READY.
 *
 *    - MP_READY implies that an MP_CAPABLE connection has been confirmed as
 *	an MPTCP connection.  See notes above.
 *
 *    - MP_DEGRADED implies that the connection has lost its MPTCP capabilities
 *	but data transfer on the MPTCP socket is unaffected.  Any existing
 *	PENDING subflows will be disconnected, and further attempts to connect
 *	additional subflows will be rejected.
 *
 * Note that these are per-subflow flags.  The setting and clearing of MP_READY
 * reflects the state of the MPTCP connection with regards to its multipath
 * semantics, via the MPTCPF_JOIN_READY flag.  Until that flag is set (meaning
 * until at least a subflow is marked with MP_READY), further connectx(2)
 * attempts to join will be queued.  When the flag is cleared (after it has
 * been set), further connectx(2) will fail (and existing queued ones will be
 * aborted) and the MPTCP connection loses all of its multipath semantics.
 *
 * Keep in sync with bsd/dev/dtrace/scripts/mptcp.d.
 */
#define MPTSF_ATTACHED          0x00000001      /* attached to MPTCP PCB */
#define MPTSF_CONNECTING        0x00000002      /* connection was attempted */
#define MPTSF_CONNECT_PENDING   0x00000004      /* will connect when MPTCP is ready */
#define MPTSF_CONNECTED         0x00000008      /* connection is established */
#define MPTSF_DISCONNECTING     0x00000010      /* disconnection was attempted */
#define MPTSF_DISCONNECTED      0x00000020      /* has been disconnected */
#define MPTSF_MP_CAPABLE        0x00000040      /* connected as a MPTCP subflow */
#define MPTSF_MP_READY          0x00000080      /* MPTCP has been confirmed */
#define MPTSF_MP_DEGRADED       0x00000100      /* has lost its MPTCP capabilities */
#define MPTSF_PREFERRED         0x00000200      /* primary/preferred subflow */
#define MPTSF_SOPT_OLDVAL       0x00000400      /* old option value is valid */
#define MPTSF_SOPT_INPROG       0x00000800      /* sosetopt in progress */
#define MPTSF_FAILINGOVER       0x00001000      /* subflow not used for output */
#define MPTSF_ACTIVE            0x00002000      /* subflow currently in use */
#define MPTSF_MPCAP_CTRSET      0x00004000      /* mpcap counter */
#define MPTSF_CLOSED            0x00008000      /* soclose_locked has been called on this subflow */
#define MPTSF_TFO_REQD          0x00010000      /* TFO requested */
#define MPTSF_CLOSE_REQD        0x00020000      /* A close has been requested from NECP */
#define MPTSF_INITIAL_SUB       0x00040000      /* This is the initial subflow */
#define MPTSF_READ_STALL        0x00080000      /* A read-stall has been detected */
#define MPTSF_WRITE_STALL       0x00100000      /* A write-stall has been detected */
#define MPTSF_FULLY_ESTABLISHED 0x00200000      /* Subflow is fully established and it has been confirmed
	                                         * whether or not it supports MPTCP.
	                                         * No need for further middlebox-detection.
	                                         */
#define MPTSF_CELLICON_SET      0x00400000      /* This subflow set the cellicon */

#define MPTSF_BITS \
	"\020\1ATTACHED\2CONNECTING\3PENDING\4CONNECTED\5DISCONNECTING" \
	"\6DISCONNECTED\7MP_CAPABLE\10MP_READY\11MP_DEGRADED" \
	"\12PREFERRED\13SOPT_OLDVAL" \
	"\14SOPT_INPROG\15FAILINGOVER\16ACTIVE\17MPCAP_CTRSET" \
	"\20CLOSED\21TFO_REQD\22CLOSEREQD\23INITIALSUB\24READ_STALL" \
	"\25WRITE_STALL\26CONFIRMED"

/*
 * MPTCP states
 * Keep in sync with bsd/dev/dtrace/mptcp.d
 */
typedef enum mptcp_state {
	MPTCPS_CLOSED           = 0,    /* closed */
	MPTCPS_LISTEN           = 1,    /* not yet implemented */
	MPTCPS_ESTABLISHED      = 2,    /* MPTCP connection established */
	MPTCPS_CLOSE_WAIT       = 3,    /* rcvd DFIN, waiting for close */
	MPTCPS_FIN_WAIT_1       = 4,    /* have closed, sent DFIN */
	MPTCPS_CLOSING          = 5,    /* closed xchd DFIN, waiting DFIN ACK */
	MPTCPS_LAST_ACK         = 6,    /* had DFIN and close; await DFIN ACK */
	MPTCPS_FIN_WAIT_2       = 7,    /* have closed, DFIN is acked */
	MPTCPS_TIME_WAIT        = 8,    /* in 2*MSL quiet wait after close */
	MPTCPS_TERMINATE        = 9,    /* terminal state */
} mptcp_state_t;

typedef u_int64_t       mptcp_key_t;
typedef u_int32_t       mptcp_token_t;
typedef u_int8_t        mptcp_addr_id;


/* Address ID list */
struct mptcp_subf_auth_entry {
	LIST_ENTRY(mptcp_subf_auth_entry) msae_next;
	u_int32_t       msae_laddr_rand;        /* Local nonce */
	u_int32_t       msae_raddr_rand;        /* Remote nonce */
	mptcp_addr_id   msae_laddr_id;          /* Local addr ID */
	mptcp_addr_id   msae_raddr_id;          /* Remote addr ID */
};

/*
 * MPTCP Protocol Control Block
 *
 * Protected by per-MPTCP mpt_lock.
 * Keep in sync with bsd/dev/dtrace/scripts/mptcp.d.
 */
struct mptcb {
	struct mptses   *mpt_mpte;              /* back ptr to MPTCP session */
	mptcp_state_t   mpt_state;              /* MPTCP state */
	uint32_t       mpt_flags;              /* see flags below */
	uint32_t       mpt_version;            /* MPTCP proto version */
	int             mpt_softerror;          /* error not yet reported */
	/*
	 * Authentication and metadata invariants
	 */
	mptcp_key_t     mpt_localkey;           /* in network byte order */
	mptcp_key_t     mpt_remotekey;          /* in network byte order */
	mptcp_token_t   mpt_localtoken;         /* HMAC SHA1 of local key */
	mptcp_token_t   mpt_remotetoken;        /* HMAC SHA1 of remote key */

	/*
	 * Timer vars for scenarios where subflow level acks arrive, but
	 * Data ACKs do not.
	 */
	int             mpt_rxtshift;           /* num of consecutive retrans */
	uint32_t        mpt_rxtstart;           /* time at which rxt started */
	uint64_t        mpt_rtseq;              /* seq # being tracked */
	uint32_t        mpt_timer_vals;         /* timer related values */
	uint32_t        mpt_timewait;           /* timewait */
	/*
	 * Sending side
	 */
	uint64_t        mpt_snduna;             /* DSN of last unacked byte */
	uint64_t        mpt_sndnxt;             /* DSN of next byte to send */
	uint64_t        mpt_sndmax;             /* DSN of max byte sent */
	uint64_t        mpt_local_idsn;         /* First byte's DSN */
	uint32_t        mpt_sndwnd;
	uint64_t        mpt_sndwl1;
	uint64_t        mpt_sndwl2;
	/*
	 * Receiving side
	 */
	uint64_t        mpt_rcvnxt;             /* Next expected DSN */
	uint64_t        mpt_remote_idsn;        /* Peer's IDSN */
	uint32_t        mpt_rcvwnd;
	uint32_t        mpt_rcvadv;
	LIST_HEAD(, mptcp_subf_auth_entry) mpt_subauth_list; /* address IDs */
	/*
	 * Fastclose
	 */
	uint64_t        mpt_dsn_at_csum_fail;   /* MPFail Opt DSN */
	uint32_t        mpt_ssn_at_csum_fail;   /* MPFail Subflow Seq */
	/*
	 * Zombie handling
	 */
#define MPT_GC_TICKS            (30)
#define MPT_GC_TICKS_FAST       (10)
	int32_t         mpt_gc_ticks;           /* Used for zombie deletion */

	uint32_t        mpt_notsent_lowat;      /* TCP_NOTSENT_LOWAT support */
	uint32_t        mpt_peer_version;       /* Version from peer */

	struct tsegqe_head      mpt_segq;
	uint16_t        mpt_reassqlen;          /* length of reassembly queue */
};

/* valid values for mpt_flags (see also notes on mpts_flags above) */
#define MPTCPF_CHECKSUM                 0x001   /* checksum DSS option */
#define MPTCPF_FALLBACK_TO_TCP          0x002   /* Fallback to TCP */
#define MPTCPF_JOIN_READY               0x004   /* Ready to start 2 or more subflows */
#define MPTCPF_RECVD_MPFAIL             0x008   /* Received MP_FAIL option */
#define MPTCPF_SND_64BITDSN             0x010   /* Send full 64-bit DSN */
#define MPTCPF_SND_64BITACK             0x020   /* Send 64-bit ACK response */
#define MPTCPF_RCVD_64BITACK            0x040   /* Received 64-bit Data ACK */
#define MPTCPF_POST_FALLBACK_SYNC       0x080   /* Post fallback resend data */
#define MPTCPF_FALLBACK_HEURISTIC       0x100   /* Send SYN without MP_CAPABLE due to heuristic */
#define MPTCPF_HEURISTIC_TRAC           0x200   /* Tracked this connection in the heuristics as a failure */
#define MPTCPF_REASS_INPROG             0x400   /* Reassembly is in progress */
#define MPTCPF_UNICAST_IP               0x800

#define MPTCPF_BITS \
	"\020\1CHECKSUM\2FALLBACK_TO_TCP\3JOIN_READY\4RECVD_MPFAIL" \
	"\5SND_64BITDSN\6SND_64BITACK\7RCVD_64BITACK\10POST_FALLBACK_SYNC" \
	"\11FALLBACK_HEURISTIC\12HEURISTIC_TRAC\13REASS_INPROG"

/* valid values for mpt_timer_vals */
#define MPTT_REXMT              0x01    /* Starting Retransmit Timer */
#define MPTT_TW                 0x02    /* Starting Timewait Timer */
#define MPTT_FASTCLOSE          0x04    /* Starting Fastclose wait timer */

/* events for close FSM */
#define MPCE_CLOSE              0x1
#define MPCE_RECV_DATA_ACK      0x2
#define MPCE_RECV_DATA_FIN      0x4

/* mptcb manipulation */
static inline struct mptcb *
tptomptp(struct tcpcb *tp)
{
	return tp->t_mptcb;
}

/*
 * MPTCP control block and state structures are allocated along with
 * the MP protocol control block; the folllowing represents the layout.
 */
struct mpp_mtp {
	struct mppcb            mpp;            /* Multipath PCB */
	struct mptses           mpp_ses;        /* MPTCP session */
	struct mptcb            mtcb;           /* MPTCP PCB */
};

#ifdef SYSCTL_DECL
SYSCTL_DECL(_net_inet_mptcp);
#endif /* SYSCTL_DECL */

extern struct mppcbinfo mtcbinfo;
extern struct pr_usrreqs mptcp_usrreqs;
extern os_log_t mptcp_log_handle;

/* Encryption algorithm related definitions */
#define SHA1_TRUNCATED          8

/* MPTCP Debugging Levels */
#define MPTCP_LOGLVL_NONE       0x0     /* No debug logging */
#define MPTCP_LOGLVL_ERR        0x1     /* Errors in execution are logged */
#define MPTCP_LOGLVL_LOG        0x2     /* Important logs */
#define MPTCP_LOGLVL_VERBOSE    0x4     /* Verbose logs */

/* MPTCP sub-components for debug logging */
#define MPTCP_NO_DBG            0x00    /* No areas are logged */
#define MPTCP_STATE_DBG         0x01    /* State machine logging */
#define MPTCP_SOCKET_DBG        0x02    /* Socket call logging */
#define MPTCP_SENDER_DBG        0x04    /* Sender side logging */
#define MPTCP_RECEIVER_DBG      0x08    /* Receiver logging */
#define MPTCP_EVENTS_DBG        0x10    /* Subflow events logging */

/* Mask to obtain 32-bit portion of data sequence number */
#define MPTCP_DATASEQ_LOW32_MASK        (0xffffffff)
#define MPTCP_DATASEQ_LOW32(seq)        (seq & MPTCP_DATASEQ_LOW32_MASK)

/* Mask to obtain upper 32-bit portion of data sequence number */
#define MPTCP_DATASEQ_HIGH32_MASK       (0xffffffff00000000)
#define MPTCP_DATASEQ_HIGH32(seq)       (seq & MPTCP_DATASEQ_HIGH32_MASK)

/* Mask to obtain 32-bit portion of data ack */
#define MPTCP_DATAACK_LOW32_MASK        (0xffffffff)
#define MPTCP_DATAACK_LOW32(ack)        (ack & MPTCP_DATAACK_LOW32_MASK)

/* Mask to obtain upper 32-bit portion of data ack */
#define MPTCP_DATAACK_HIGH32_MASK       (0xffffffff00000000)
#define MPTCP_DATAACK_HIGH32(ack)       (ack & MPTCP_DATAACK_HIGH32_MASK)

/*
 * x is the 64-bit data sequence number, y the 32-bit data seq number to be
 * extended. z is y extended to the appropriate 64-bit value.
 * This algorithm is based on the fact that subflow level window sizes are
 * at the maximum 2**30 (in reality, they are a lot lesser). A high throughput
 * application sending on a large number of subflows can in theory have very
 * large MPTCP level send and receive windows. In which case, 64 bit DSNs
 * must be sent in place of 32 bit DSNs on wire. For us, with 2 subflows at
 * 512K each, sequence wraparound detection can be done by checking whether
 * the 32-bit value obtained on wire is 2**31 bytes apart from the stored
 * lower 32-bits of the Data Sequence Number. Bogus DSNs are dropped by
 * comparing against rwnd. Bogus DSNs within rwnd cannot be protected against
 * and are as weak as bogus TCP sequence numbers.
 */
#define MPTCP_EXTEND_DSN(x, y, z) {                                     \
	if ((MPTCP_DATASEQ_LOW32(x) > y) &&                             \
	    ((((u_int32_t)MPTCP_DATASEQ_LOW32(x)) - (u_int32_t)y) >=    \
	    (u_int32_t)(1U << 31))) {                                    \
	/* \
	 * y wrapped around and x and y are 2**31 bytes  apart \
	 */                                                             \
	        z = MPTCP_DATASEQ_HIGH32(x) + 0x100000000;              \
	        z |= y;                                                 \
	} else if ((MPTCP_DATASEQ_LOW32(x) < y) &&                      \
	    (((u_int32_t)y -                                            \
	    ((u_int32_t)MPTCP_DATASEQ_LOW32(x))) >=                     \
	    (u_int32_t)(1U << 31))) {                                    \
	/* \
	 * x wrapped around and x and y are 2**31 apart \
	 */                                                             \
	        z = MPTCP_DATASEQ_HIGH32(x) - 0x100000000;              \
	        z |= y;                                                 \
	} else {                                                        \
	        z = MPTCP_DATASEQ_HIGH32(x) | y;                        \
	}                                                               \
}

#define mptcplog(x, y, z)       do {                                    \
	if ((mptcp_dbg_area & y) && (mptcp_dbg_level & z))              \
	        log x;                                                  \
} while (0)

extern int mptcp_enable;        /* Multipath TCP */
extern int mptcp_mpcap_retries; /* Multipath TCP retries */
extern int mptcp_join_retries;  /* Multipath TCP Join retries */
extern int mptcp_dss_csum;      /* Multipath DSS Option checksum */
extern int mptcp_fail_thresh;   /* Multipath failover thresh of retransmits */
extern int mptcp_subflow_keeptime; /* Multipath subflow TCP_KEEPALIVE opt */
extern uint32_t mptcp_dbg_level;        /* Multipath TCP debugging level */
extern uint32_t mptcp_dbg_area; /* Multipath TCP debugging area */
extern int mptcp_developer_mode;        /* Allow aggregation mode */
extern uint32_t mptcp_cellicon_refcount;

#define MPTCP_CELLICON_TOGGLE_RATE      (5 * TCP_RETRANSHZ) /* Only toggle every 5 seconds */

extern int tcp_jack_rxmt;       /* Join ACK retransmission value in msecs */

__BEGIN_DECLS
extern void mptcp_init(struct protosw *, struct domain *);
extern int mptcp_ctloutput(struct socket *, struct sockopt *);
extern int mptcp_session_create(struct mppcb *);
extern boolean_t mptcp_ok_to_create_subflows(struct mptcb *mp_tp);
extern void mptcp_check_subflows_and_add(struct mptses *mpte);
extern void mptcp_check_subflows_and_remove(struct mptses *mpte);
extern void mptcpstats_inc_switch(struct mptses *mpte, const struct mptsub *mpts);
extern void mptcpstats_update(struct mptcp_itf_stats *stats, const struct mptsub *mpts);
extern int mptcpstats_get_index_by_ifindex(struct mptcp_itf_stats *stats, int ifindex, boolean_t create);
extern struct mptses *mptcp_drop(struct mptses *, struct mptcb *, int);
extern struct mptses *mptcp_close(struct mptses *, struct mptcb *);
extern int mptcp_lock(struct socket *, int, void *);
extern int mptcp_unlock(struct socket *, int, void *);
extern lck_mtx_t *mptcp_getlock(struct socket *, int);
extern void mptcp_subflow_workloop(struct mptses *);

extern void mptcp_sched_create_subflows(struct mptses *);

extern void mptcp_finish_usrclosed(struct mptses *mpte);
extern struct mptopt *mptcp_sopt_alloc(int);
extern const char *mptcp_sopt2str(int, int);
extern void mptcp_sopt_free(struct mptopt *);
extern void mptcp_sopt_insert(struct mptses *, struct mptopt *);
extern void mptcp_sopt_remove(struct mptses *, struct mptopt *);
extern struct mptopt *mptcp_sopt_find(struct mptses *, struct sockopt *);

extern int mptcp_subflow_add(struct mptses *, struct sockaddr *,
    struct sockaddr *, uint32_t, sae_connid_t *);
extern void mptcp_subflow_del(struct mptses *, struct mptsub *);

extern void mptcp_handle_input(struct socket *so);
#define MPTCP_SUBOUT_PROBING    0x01
extern int mptcp_subflow_output(struct mptses *mpte, struct mptsub *mpts, int flags);
extern void mptcp_clean_reinjectq(struct mptses *mpte);
extern void mptcp_subflow_shutdown(struct mptses *, struct mptsub *);
extern void mptcp_subflow_disconnect(struct mptses *, struct mptsub *);
extern int mptcp_subflow_sosetopt(struct mptses *, struct mptsub *,
    struct mptopt *);
extern int mptcp_subflow_sogetopt(struct mptses *, struct socket *,
    struct mptopt *);

extern void mptcp_input(struct mptses *, struct mbuf *);
extern boolean_t mptcp_can_send_more(struct mptcb *mp_tp, boolean_t ignore_reinject);
extern int mptcp_output(struct mptses *);
extern void mptcp_close_fsm(struct mptcb *, uint32_t);

extern void mptcp_hmac_sha1(mptcp_key_t, mptcp_key_t, u_int32_t, u_int32_t,
    u_char*);
extern void mptcp_get_hmac(mptcp_addr_id, struct mptcb *, u_char *);
extern void mptcp_get_rands(mptcp_addr_id, struct mptcb *, u_int32_t *,
    u_int32_t *);
extern void mptcp_set_raddr_rand(mptcp_addr_id, struct mptcb *, mptcp_addr_id,
    u_int32_t);
extern int mptcp_init_remote_parms(struct mptcb *);
extern boolean_t mptcp_ok_to_keepalive(struct mptcb *);
extern void mptcp_insert_dsn(struct mppcb *, struct mbuf *);
extern void mptcp_output_getm_dsnmap32(struct socket *so, int off,
    uint32_t *dsn, uint32_t *relseq,
    uint16_t *data_len, uint16_t *dss_csum);
extern void mptcp_output_getm_dsnmap64(struct socket *so, int off,
    uint64_t *dsn, uint32_t *relseq,
    uint16_t *data_len, uint16_t *dss_csum);
extern void mptcp_act_on_txfail(struct socket *);
extern struct mptsub *mptcp_get_subflow(struct mptses *mpte, struct mptsub **preferred);
extern int mptcp_get_map_for_dsn(struct socket *, u_int64_t, u_int32_t *);
extern int32_t mptcp_adj_sendlen(struct socket *so, int32_t off);
extern void mptcp_sbrcv_grow(struct mptcb *mp_tp);
extern int32_t mptcp_sbspace(struct mptcb *);
extern void mptcp_notify_mpready(struct socket *);
extern void mptcp_notify_mpfail(struct socket *);
extern void mptcp_notify_close(struct socket *);
extern boolean_t mptcp_no_rto_spike(struct socket*);
extern int mptcp_set_notsent_lowat(struct mptses *mpte, int optval);
extern u_int32_t mptcp_get_notsent_lowat(struct mptses *mpte);
extern int mptcp_notsent_lowat_check(struct socket *so);
extern void mptcp_ask_symptoms(struct mptses *mpte);
extern void mptcp_control_register(void);
extern int mptcp_is_wifi_unusable_for_session(struct mptses *mpte);
extern boolean_t symptoms_is_wifi_lossy(void);
extern void mptcp_ask_for_nat64(struct ifnet *ifp);
extern void mptcp_session_necp_cb(void *, int, uint32_t, uint32_t, bool *);
extern struct sockaddr *mptcp_get_session_dst(struct mptses *mpte,
    boolean_t has_v6, boolean_t has_v4);
extern void mptcp_set_restrictions(struct socket *mp_so);
extern void mptcp_clear_cellicon(void);
extern void mptcp_unset_cellicon(struct mptses *mpte, struct mptsub *mpts, uint32_t val);
extern void mptcp_reset_rexmit_state(struct tcpcb *tp);
extern void mptcp_reset_keepalive(struct tcpcb *tp);
extern int mptcp_validate_csum(struct tcpcb *tp, struct mbuf *m, uint64_t dsn,
    uint32_t sseq, uint16_t dlen, uint16_t csum,
    uint16_t dfin);
__END_DECLS

#endif /* BSD_KERNEL_PRIVATE */
#ifdef PRIVATE

typedef struct mptcp_flow {
	size_t                  flow_len;
	size_t                  flow_tcpci_offset;
	uint32_t                flow_flags;
	sae_connid_t            flow_cid;
	struct sockaddr_storage flow_src;
	struct sockaddr_storage flow_dst;
	uint32_t                flow_relseq;    /* last subflow rel seq# */
	int32_t                 flow_soerror;   /* subflow level error */
	uint32_t                flow_probecnt;  /* number of probes sent */
	conninfo_tcp_t          flow_ci;        /* must be the last field */
} mptcp_flow_t;

typedef struct conninfo_mptcp {
	size_t          mptcpci_len;
	size_t          mptcpci_flow_offset;    /* offsetof first flow */
	size_t          mptcpci_nflows;         /* number of subflows */
	uint32_t        mptcpci_state;          /* MPTCP level state */
	uint32_t        mptcpci_mpte_flags;     /* Session flags */
	uint32_t        mptcpci_flags;          /* MPTCB flags */
	uint32_t        mptcpci_ltoken;         /* local token */
	uint32_t        mptcpci_rtoken;         /* remote token */
	uint32_t        mptcpci_notsent_lowat;  /* NOTSENT_LOWAT */

	/* Send side */
	uint64_t        mptcpci_snduna;         /* DSN of last unacked byte */
	uint64_t        mptcpci_sndnxt;         /* DSN of next byte to send */
	uint64_t        mptcpci_sndmax;         /* DSN of max byte sent */
	uint64_t        mptcpci_lidsn;          /* Local IDSN */
	uint32_t        mptcpci_sndwnd;         /* Send window snapshot */

	/* Receive side */
	uint64_t        mptcpci_rcvnxt;         /* Next expected DSN */
	uint64_t        mptcpci_rcvatmark;      /* Session level rcvnxt */
	uint64_t        mptcpci_ridsn;          /* Peer's IDSN */
	uint32_t        mptcpci_rcvwnd;         /* Receive window */

	uint8_t         mptcpci_mpte_addrid;    /* last addr id */

	mptcp_flow_t    mptcpci_flows[1];
} conninfo_mptcp_t;

/* Use SymptomsD notifications of wifi and cell status in subflow selection */
#define MPTCP_KERN_CTL_NAME    "com.apple.network.advisory"
typedef struct symptoms_advisory {
	union {
		uint32_t        sa_nwk_status_int;
		struct {
			union {
#define SYMPTOMS_ADVISORY_NOCOMMENT     0x0000
#define SYMPTOMS_ADVISORY_USEAPP        0xFFFF /* Very ugly workaround to avoid breaking backwards compatibility - ToDo: Fix it in +1 */
				uint16_t        sa_nwk_status;
				struct {
#define SYMPTOMS_ADVISORY_WIFI_BAD     0x01
#define SYMPTOMS_ADVISORY_WIFI_OK      0x02
					uint8_t sa_wifi_status;
#define SYMPTOMS_ADVISORY_CELL_BAD     0x01
#define SYMPTOMS_ADVISORY_CELL_OK      0x02
					uint8_t sa_cell_status;
				};
			};
			uint16_t        sa_unused;
		};
	};
} symptoms_advisory_t;

#define MPTCP_TARGET_BASED_RSSI_THRESHOLD -75
struct mptcp_symptoms_answer {
	struct symptoms_advisory advisory;
	uuid_t  uuid;
	int32_t rssi;
};

struct mptcp_symptoms_ask_uuid {
	uint32_t        cmd;
#define MPTCP_SYMPTOMS_ASK_UUID         1
	uuid_t          uuid;
	uint32_t        priority;
#define MPTCP_SYMPTOMS_UNKNOWN          0
#define MPTCP_SYMPTOMS_BACKGROUND       1
#define MPTCP_SYMPTOMS_FOREGROUND       2
};

struct kev_mptcp_data {
	int value;
};

#endif /* PRIVATE */
#endif /* _NETINET_MPTCP_VAR_H_ */
