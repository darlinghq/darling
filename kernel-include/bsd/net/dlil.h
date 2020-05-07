/*
 * Copyright (c) 1999-2019 Apple Inc. All rights reserved.
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
#ifndef DLIL_H
#define DLIL_H
#ifdef KERNEL

#include <sys/kernel_types.h>
#include <net/kpi_interface.h>

enum {
	BPF_TAP_DISABLE,
	BPF_TAP_INPUT,
	BPF_TAP_OUTPUT,
	BPF_TAP_INPUT_OUTPUT
};

/*
 * DLIL_DESC_ETYPE2 - native_type must point to 2 byte ethernet raw protocol,
 *                    variants.native_type_length must be set to 2
 * DLIL_DESC_SAP - native_type must point to 3 byte SAP protocol
 *                 variants.native_type_length must be set to 3
 * DLIL_DESC_SNAP - native_type must point to 5 byte SNAP protocol
 *                  variants.native_type_length must be set to 5
 *
 * All protocols must be in Network byte order.
 *
 * Future interface families may define more protocol types they know about.
 * The type implies the offset and context of the protocol data at native_type.
 * The length of the protocol data specified at native_type must be set in
 * variants.native_type_length.
 */
/* Ethernet specific types */
#define DLIL_DESC_ETYPE2        4
#define DLIL_DESC_SAP           5
#define DLIL_DESC_SNAP          6

#ifdef KERNEL_PRIVATE
#include <net/if.h>
#include <net/if_var.h>
#include <net/classq/classq.h>
#include <net/flowadv.h>
#include <sys/kern_event.h>
#include <kern/thread.h>
#include <kern/locks.h>

#ifdef BSD_KERNEL_PRIVATE
/* Operations on timespecs. */
#define net_timerclear(tvp)     (tvp)->tv_sec = (tvp)->tv_nsec = 0

#define net_timerisset(tvp)     ((tvp)->tv_sec || (tvp)->tv_nsec)

#define net_timercmp(tvp, uvp, cmp)                                     \
	(((tvp)->tv_sec == (uvp)->tv_sec) ?                             \
	((tvp)->tv_nsec cmp (uvp)->tv_nsec) :                           \
	((tvp)->tv_sec cmp (uvp)->tv_sec))

#define net_timeradd(tvp, uvp, vvp) do {                                \
	(vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;                  \
	(vvp)->tv_nsec = (tvp)->tv_nsec + (uvp)->tv_nsec;               \
	if ((vvp)->tv_nsec >= (long)NSEC_PER_SEC) {                     \
	        (vvp)->tv_sec++;                                        \
	        (vvp)->tv_nsec -= NSEC_PER_SEC;                         \
	}                                                               \
} while (0)

#define net_timersub(tvp, uvp, vvp) do {                                \
	(vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;                  \
	(vvp)->tv_nsec = (tvp)->tv_nsec - (uvp)->tv_nsec;               \
	if ((vvp)->tv_nsec < 0) {                                       \
	        (vvp)->tv_sec--;                                        \
	        (vvp)->tv_nsec += NSEC_PER_SEC;                         \
	}                                                               \
} while (0)

#define net_timerusec(tvp, nsp) do {                                    \
	*(nsp) = (tvp)->tv_nsec / NSEC_PER_USEC;                        \
	if ((tvp)->tv_sec > 0)                                          \
	        *(nsp) += ((tvp)->tv_sec * USEC_PER_SEC);               \
} while (0)

#define net_timernsec(tvp, nsp) do {                                    \
	*(nsp) = (tvp)->tv_nsec;                                        \
	if ((tvp)->tv_sec > 0)                                          \
	        *(nsp) += ((tvp)->tv_sec * NSEC_PER_SEC);               \
} while (0)

#if defined(__x86_64__) || defined(__arm64__)
#define net_nsectimer(nsp, tvp) do {                                    \
	u_int64_t __nsp = *(nsp);                                       \
	net_timerclear(tvp);                                            \
	uint64_t __sec = __nsp / NSEC_PER_SEC;                          \
	(tvp)->tv_sec = (__darwin_time_t)__sec;                         \
	(tvp)->tv_nsec = (long)(__nsp - __sec * NSEC_PER_SEC);          \
} while (0)
#else /* 32 bit */
/*
 * NSEC needs to be < 2^31*10^9 to be representable in a struct timespec
 * because __darwin_time_t is 32 bit on 32-bit platforms. This bound
 * is < 2^61. We get a first approximation to convert into seconds using
 * the following values.
 * a = floor(NSEC / 2^29)
 * inv = floor(2^61 / 10^9)
 *
 * The approximation of seconds is correct or too low by 1 unit.
 * So we fix it by computing the remainder.
 */
#define net_nsectimer(nsp, tvp) do {                                    \
	u_int64_t __nsp = *(nsp);                                       \
	net_timerclear(tvp);                                            \
	uint32_t __a = (uint32_t)(__nsp >> 29);                         \
	const uint32_t __inv = 0x89705F41;                              \
	uint32_t __sec = (uint32_t)(((uint64_t)__a * __inv) >> 32);     \
	uint32_t __rem = (uint32_t)(__nsp - __sec * NSEC_PER_SEC);      \
	__sec += ((__rem >= NSEC_PER_SEC) ? 1 : 0);                     \
	(tvp)->tv_sec = (__darwin_time_t)__sec;                         \
	(tvp)->tv_nsec =                                                \
	    (long)((__rem >= NSEC_PER_SEC) ? (__rem - NSEC_PER_SEC) : __rem);   \
} while(0)
#endif /* 32 bit */

struct ifnet;
struct mbuf;
struct ether_header;
struct sockaddr_dl;
struct iff_filter;

#define DLIL_THREADNAME_LEN     32

/*
 * DLIL input thread info
 */
struct dlil_threading_info {
	decl_lck_mtx_data(, input_lck);
	lck_grp_t       *lck_grp;       /* lock group (for lock stats) */
	u_int32_t       input_waiting;  /* DLIL condition of thread */
	u_int32_t       wtot;           /* # of wakeup requests */
	char            input_name[DLIL_THREADNAME_LEN]; /* name storage */
	struct ifnet    *ifp;           /* pointer to interface */
	class_queue_t   rcvq_pkts;      /* queue of pkts */
	struct ifnet_stat_increment_param stats; /* incremental statistics */
	/*
	 * Thread affinity (workloop and DLIL threads).
	 */
	boolean_t       net_affinity;   /* affinity set is available */
	struct thread   *input_thr;     /* input thread */
	struct thread   *wloop_thr;     /* workloop thread */
	struct thread   *poll_thr;      /* poll thread */
	u_int32_t       tag;            /* affinity tag */
#if IFNET_INPUT_SANITY_CHK
	/*
	 * For debugging.
	 */
	u_int64_t       input_mbuf_cnt; /* total # of packets processed */
#endif
};

/*
 * DLIL input thread info (for main/loopback input thread)
 */
struct dlil_main_threading_info {
	struct dlil_threading_info      inp;
	class_queue_t                   lo_rcvq_pkts; /* queue of lo0 pkts */
};

/*
 * The following are shared with kpi_protocol.c so that it may wakeup
 * the input thread to run through packets queued for protocol input.
 */
#define DLIL_INPUT_RUNNING      0x80000000
#define DLIL_INPUT_WAITING      0x40000000
#define DLIL_PROTO_REGISTER     0x20000000
#define DLIL_PROTO_WAITING      0x10000000
#define DLIL_INPUT_TERMINATE    0x08000000
#define DLIL_INPUT_TERMINATE_COMPLETE   0x04000000

/*
 * Flags for dlil_attach_filter()
 */
#define DLIL_IFF_TSO            0x01    /* Interface filter supports TSO */
#define DLIL_IFF_INTERNAL       0x02    /* Apple internal -- do not count towards stats */

/* Input poll interval definitions */
#define IF_RXPOLL_INTERVALTIME_MIN      (1ULL * 1000)           /* 1 us */
#define IF_RXPOLL_INTERVALTIME          (1ULL * 1000 * 1000)    /* 1 ms */

extern int dlil_verbose;
extern uint32_t hwcksum_dbg;
extern uint32_t hwcksum_tx;
extern uint32_t hwcksum_rx;
extern struct dlil_threading_info *dlil_main_input_thread;
extern unsigned int net_rxpoll;
extern uint32_t if_rxpoll;
extern uint32_t if_rxpoll_decay;
extern uint32_t if_rxpoll_interval_pkts;
extern uint32_t if_rcvq_maxlen;

extern void dlil_init(void);

extern errno_t ifp_if_ioctl(struct ifnet *, unsigned long, void *);
extern errno_t ifp_if_output(struct ifnet *, struct mbuf *);
extern void ifp_if_start(struct ifnet *);

extern errno_t dlil_set_bpf_tap(ifnet_t, bpf_tap_mode, bpf_packet_func);

/*
 * Send arp internal bypasses the check for IPv4LL.
 */
extern errno_t dlil_send_arp_internal(ifnet_t, u_int16_t,
    const struct sockaddr_dl *, const struct sockaddr *,
    const struct sockaddr_dl *, const struct sockaddr *);

/*
 * The following constants are used with the net_thread_mark_apply and
 * net_thread_is_unmarked functions to control the bits in the uu_network_marks
 * field of the uthread structure.
 */
#define NET_THREAD_HELD_PF      0x1     /* thread is holding PF lock */
#define NET_THREAD_HELD_DOMAIN  0x2     /* thread is holding domain_proto_mtx */
#define NET_THREAD_CKREQ_LLADDR 0x4     /* thread reqs MACF check for LLADDR */

/*
 * net_thread_marks_t is a pointer to a phantom structure type used for
 * manipulating the uthread:uu_network_marks field.  As an example...
 *
 *   static const u_int32_t bits = NET_THREAD_CKREQ_LLADDR;
 *   struct uthread *uth = get_bsdthread_info(current_thread());
 *
 *   net_thread_marks_t marks = net_thread_marks_push(bits);
 *   VERIFY((uth->uu_network_marks & NET_THREAD_CKREQ_LLADDR) != 0);
 *   net_thread_marks_pop(marks);
 *
 * The net_thread_marks_push() function returns an encoding of the bits
 * that were changed from zero to one in the uu_network_marks field. When
 * the net_thread_marks_pop() function later processes that value, it
 * resets the bits to their previous value.
 *
 * The net_thread_unmarks_push() and net_thread_unmarks_pop() functions
 * are similar to net_thread_marks_push() and net_thread_marks_pop() except
 * they clear the marks bits in the guarded section rather than set them.
 *
 * The net_thread_is_marked() and net_thread_is_unmarked() functions return
 * the subset of the bits that are currently set or cleared (respectively)
 * in the uthread:uu_network_marks field.
 *
 * Finally, the value of the net_thread_marks_none constant is provided for
 * comparing for equality with the value returned when no bits in the marks
 * field are changed by the push.
 *
 * It is not significant that a value of type net_thread_marks_t may
 * compare as equal to the NULL pointer.
 */
struct net_thread_marks;
typedef const struct net_thread_marks *net_thread_marks_t;

extern const net_thread_marks_t net_thread_marks_none;

extern net_thread_marks_t net_thread_marks_push(u_int32_t);
extern net_thread_marks_t net_thread_unmarks_push(u_int32_t);
extern void net_thread_marks_pop(net_thread_marks_t);
extern void net_thread_unmarks_pop(net_thread_marks_t);
extern u_int32_t net_thread_is_marked(u_int32_t);
extern u_int32_t net_thread_is_unmarked(u_int32_t);

extern int dlil_output(ifnet_t, protocol_family_t, mbuf_t, void *,
    const struct sockaddr *, int, struct flowadv *);

extern void dlil_input_packet_list(struct ifnet *, struct mbuf *);
extern void dlil_input_packet_list_extended(struct ifnet *, struct mbuf *,
    u_int32_t, ifnet_model_t);

extern errno_t dlil_resolve_multi(struct ifnet *,
    const struct sockaddr *, struct sockaddr *, size_t);

extern errno_t dlil_send_arp(ifnet_t, u_int16_t, const struct sockaddr_dl *,
    const struct sockaddr *, const struct sockaddr_dl *,
    const struct sockaddr *, u_int32_t);

extern int dlil_attach_filter(ifnet_t, const struct iff_filter *,
    interface_filter_t *, u_int32_t);
extern void dlil_detach_filter(interface_filter_t);

extern void dlil_proto_unplumb_all(ifnet_t);

extern int dlil_post_msg(struct ifnet *, u_int32_t, u_int32_t,
    struct net_event_data *, u_int32_t);

extern void dlil_post_sifflags_msg(struct ifnet *);

extern int dlil_post_complete_msg(struct ifnet *, struct kev_msg *);

extern int dlil_alloc_local_stats(struct ifnet *);

extern void ifnet_filter_update_tso(boolean_t filter_enable);
extern errno_t dlil_rxpoll_validate_params(struct ifnet_poll_params *);
extern void dlil_rxpoll_update_params(struct ifnet *,
    struct ifnet_poll_params *);
extern void ifnet_poll(struct ifnet *);
extern errno_t ifnet_input_poll(struct ifnet *, struct mbuf *,
    struct mbuf *, const struct ifnet_stat_increment_param *);


/*
 * dlil_if_acquire is obsolete. Use ifnet_allocate.
 */
extern int dlil_if_acquire(u_int32_t, const void *, size_t, const char *, struct ifnet **);
/*
 * dlil_if_release is obsolete. The equivalent is called automatically when
 * an interface is detached.
 */
extern void dlil_if_release(struct ifnet *ifp);

extern errno_t dlil_if_ref(struct ifnet *);
extern errno_t dlil_if_free(struct ifnet *);

extern int dlil_node_present(struct ifnet *, struct sockaddr *, int32_t, int,
    int, u_int8_t[48]);
extern void dlil_node_absent(struct ifnet *, struct sockaddr *);
extern int dlil_node_present_v2(struct ifnet *, struct sockaddr *, struct sockaddr_dl *, int32_t, int,
    int, u_int8_t[48]);

extern const void *dlil_ifaddr_bytes(const struct sockaddr_dl *, size_t *,
    kauth_cred_t *);

extern void dlil_report_issues(struct ifnet *, u_int8_t[DLIL_MODIDLEN],
    u_int8_t[DLIL_MODARGLEN]);

#define PROTO_HASH_SLOTS        5

extern int proto_hash_value(u_int32_t);

extern const char *dlil_kev_dl_code_str(u_int32_t);

extern errno_t dlil_rxpoll_set_params(struct ifnet *,
    struct ifnet_poll_params *, boolean_t);
extern errno_t dlil_rxpoll_get_params(struct ifnet *,
    struct ifnet_poll_params *);

extern errno_t dlil_output_handler(struct ifnet *, struct mbuf *);
extern errno_t dlil_input_handler(struct ifnet *, struct mbuf *,
    struct mbuf *, const struct ifnet_stat_increment_param *,
    boolean_t, struct thread *);


/*
 * This is mostly called from the context of the DLIL input thread;
 * because of that there is no need for atomic operations.
 */
__attribute__((always_inline))
static inline void
ifp_inc_traffic_class_in(struct ifnet *ifp, struct mbuf *m)
{
	if (!(m->m_flags & M_PKTHDR)) {
		return;
	}

	switch (m_get_traffic_class(m)) {
	case MBUF_TC_BE:
		ifp->if_tc.ifi_ibepackets++;
		ifp->if_tc.ifi_ibebytes += m->m_pkthdr.len;
		break;
	case MBUF_TC_BK:
		ifp->if_tc.ifi_ibkpackets++;
		ifp->if_tc.ifi_ibkbytes += m->m_pkthdr.len;
		break;
	case MBUF_TC_VI:
		ifp->if_tc.ifi_ivipackets++;
		ifp->if_tc.ifi_ivibytes += m->m_pkthdr.len;
		break;
	case MBUF_TC_VO:
		ifp->if_tc.ifi_ivopackets++;
		ifp->if_tc.ifi_ivobytes += m->m_pkthdr.len;
		break;
	default:
		break;
	}

	if (mbuf_is_traffic_class_privileged(m)) {
		ifp->if_tc.ifi_ipvpackets++;
		ifp->if_tc.ifi_ipvbytes += m->m_pkthdr.len;
	}
}

/*
 * This is called from DLIL output, hence multiple threads could end
 * up modifying the statistics.  We trade off acccuracy for performance
 * by not using atomic operations here.
 */
__attribute__((always_inline))
static inline void
ifp_inc_traffic_class_out(struct ifnet *ifp, struct mbuf *m)
{
	if (!(m->m_flags & M_PKTHDR)) {
		return;
	}

	switch (m_get_traffic_class(m)) {
	case MBUF_TC_BE:
		ifp->if_tc.ifi_obepackets++;
		ifp->if_tc.ifi_obebytes += m->m_pkthdr.len;
		break;
	case MBUF_TC_BK:
		ifp->if_tc.ifi_obkpackets++;
		ifp->if_tc.ifi_obkbytes += m->m_pkthdr.len;
		break;
	case MBUF_TC_VI:
		ifp->if_tc.ifi_ovipackets++;
		ifp->if_tc.ifi_ovibytes += m->m_pkthdr.len;
		break;
	case MBUF_TC_VO:
		ifp->if_tc.ifi_ovopackets++;
		ifp->if_tc.ifi_ovobytes += m->m_pkthdr.len;
		break;
	default:
		break;
	}

	if (mbuf_is_traffic_class_privileged(m)) {
		ifp->if_tc.ifi_opvpackets++;
		ifp->if_tc.ifi_opvbytes += m->m_pkthdr.len;
	}
}
#endif /* BSD_KERNEL_PRIVATE */
#endif /* KERNEL_PRIVATE */
#endif /* KERNEL */
#endif /* DLIL_H */
