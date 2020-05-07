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
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _NETINET6_ND6_H_
#define _NETINET6_ND6_H_
#include <sys/appleapiopts.h>
#include <net/net_kev.h>

/* see net/route.h, or net/if_inarp.h */
#ifndef RTF_ANNOUNCE
#define RTF_ANNOUNCE    RTF_PROTO2
#endif

#include <sys/queue.h>

#ifdef BSD_KERNEL_PRIVATE
#include <net/flowadv.h>
#include <kern/locks.h>
#include <sys/tree.h>
#include <sys/eventhandler.h>
#include <netinet6/nd6_var.h>

struct  llinfo_nd6 {
	/*
	 * The following are protected by rnh_lock
	 */
	struct  llinfo_nd6 *ln_next;
	struct  llinfo_nd6 *ln_prev;
	struct  rtentry *ln_rt;
	/*
	 * The following are protected by rt_lock
	 */
	struct ifnet *ln_exclifp; /* excluded interface (prefix proxy) */
	struct  mbuf *ln_hold;  /* last packet until resolved/timeout */
	uint32_t ln_asked;      /* # of queries already sent for this addr */
	short   ln_state;       /* reachability state */
	short   ln_router;      /* 2^0: ND6 router bit */
	u_int32_t ln_flags;     /* flags; see below */
	u_int64_t ln_expire;    /* lifetime for NDP state transition */
	u_int64_t ln_lastused;  /* last used timestamp */
	struct  if_llreach *ln_llreach; /* link-layer reachability record */
};

/* Values for ln_flags */
#define ND6_LNF_TIMER_SKIP      0x1     /* modified by nd6_timer() */
#define ND6_LNF_IN_USE          0x2     /* currently in llinfo_nd6 list */
#endif /* BSD_KERNEL_PRIVATE */

#define ND6_LLINFO_PURGE        -3
#define ND6_LLINFO_NOSTATE      -2
/*
 * We don't need the WAITDELETE state any more, but we keep the definition
 * in a comment line instead of removing it. This is necessary to avoid
 * unintentionally reusing the value for another purpose, which might
 * affect backward compatibility with old applications.
 * (20000711 jinmei@kame.net)
 */
/* #define	ND6_LLINFO_WAITDELETE	-1 */
#define ND6_LLINFO_INCOMPLETE   0
#define ND6_LLINFO_REACHABLE    1
#define ND6_LLINFO_STALE        2
#define ND6_LLINFO_DELAY        3
#define ND6_LLINFO_PROBE        4

#ifdef BSD_KERNEL_PRIVATE

#define ND6_CACHE_STATE_TRANSITION(ln, nstate) do {\
	if (nd6_debug >= 1) {\
	        struct rtentry *ln_rt = ln != NULL ? (ln)->ln_rt : NULL; \
	        nd6log(info,\
	            "[%s:%d]: NDP cache entry changed from %s -> %s for address %s.\n",\
	            __func__,\
	            __LINE__,\
	            ndcache_state2str((ln)->ln_state),\
	            ndcache_state2str(nstate),\
	            ln_rt != NULL ? ip6_sprintf(&SIN6(rt_key(ln_rt))->sin6_addr) : "N/A");\
	}\
	if (ln != NULL)\
	        (ln)->ln_state = nstate;\
} while(0)

#define ND6_IS_LLINFO_PROBREACH(n) ((n)->ln_state > ND6_LLINFO_INCOMPLETE)
#define ND6_LLINFO_PERMANENT(n) \
	(((n)->ln_expire == 0) && ((n)->ln_state > ND6_LLINFO_INCOMPLETE))

#define ND6_EUI64_GBIT  0x01
#define ND6_EUI64_UBIT  0x02

#define ND6_EUI64_TO_IFID(in6) \
	do {(in6)->s6_addr[8] ^= ND6_EUI64_UBIT; } while (0)

#define ND6_EUI64_GROUP(in6)            ((in6)->s6_addr[8] & ND6_EUI64_GBIT)
#define ND6_EUI64_INDIVIDUAL(in6)       (!ND6_EUI64_GROUP(in6))
#define ND6_EUI64_LOCAL(in6)            ((in6)->s6_addr[8] & ND6_EUI64_UBIT)
#define ND6_EUI64_UNIVERSAL(in6)        (!ND6_EUI64_LOCAL(in6))
#define ND6_IFID_LOCAL(in6)             (!ND6_EUI64_LOCAL(in6))
#define ND6_IFID_UNIVERSAL(in6)         (!ND6_EUI64_UNIVERSAL(in6))
#endif /* BSD_KERNEL_PRIVATE */

#if !defined(BSD_KERNEL_PRIVATE)
struct nd_ifinfo {
#else
/* For binary compatibility, this structure must not change */
/* NOTE: nd_ifinfo is defined in nd6_var.h */
struct nd_ifinfo_compat {
#endif /* !BSD_KERNEL_PRIVATE */
	u_int32_t linkmtu;              /* LinkMTU */
	u_int32_t maxmtu;               /* Upper bound of LinkMTU */
	u_int32_t basereachable;        /* BaseReachableTime */
	u_int32_t reachable;            /* Reachable Time */
	u_int32_t retrans;              /* Retrans Timer */
	u_int32_t flags;                /* Flags */
	int recalctm;                   /* BaseReacable re-calculation timer */
	u_int8_t chlim;                 /* CurHopLimit */
	u_int8_t receivedra;
	/* the following 3 members are for privacy extension for addrconf */
	u_int8_t randomseed0[8]; /* upper 64 bits of SHA1 digest */
	u_int8_t randomseed1[8]; /* lower 64 bits (usually the EUI64 IFID) */
	u_int8_t randomid[8];   /* current random ID */
};

#define ND6_IFF_PERFORMNUD              0x1
#if defined(PRIVATE)

/*
 * APPLE: not used. Interface specific router advertisements are handled with a
 * specific ifnet flag: IFEF_ACCEPT_RTADVD
 */
#define ND6_IFF_ACCEPT_RTADV            0x2

/* APPLE: NOT USED not related to ND. */
#define ND6_IFF_PREFER_SOURCE           0x4

/* IPv6 operation is disabled due to * DAD failure.  (XXX: not ND-specific) */
#define ND6_IFF_IFDISABLED              0x8

#define ND6_IFF_DONT_SET_IFROUTE        0x10 /* NOT USED */
#endif /* PRIVATE */
#define ND6_IFF_PROXY_PREFIXES          0x20
#define ND6_IFF_IGNORE_NA               0x40
#if defined(PRIVATE)
#define ND6_IFF_INSECURE                0x80
#endif
#define ND6_IFF_REPLICATED              0x100   /* sleep proxy registered */
#define ND6_IFF_DAD                     0x200   /* Perform DAD on the interface */

struct in6_nbrinfo {
	char ifname[IFNAMSIZ];  /* if name, e.g. "en0" */
	struct in6_addr addr;   /* IPv6 address of the neighbor */
	long    asked;          /* # of queries already sent for this addr */
	int     isrouter;       /* if it acts as a router */
	int     state;          /* reachability state */
	int     expire;         /* lifetime for NDP state transition */
};

#if defined(BSD_KERNEL_PRIVATE)
struct in6_nbrinfo_32 {
	char ifname[IFNAMSIZ];
	struct in6_addr addr;
	u_int32_t asked;
	int     isrouter;
	int     state;
	int     expire;
};

struct in6_nbrinfo_64 {
	char ifname[IFNAMSIZ];
	struct in6_addr addr;
	long    asked;
	int     isrouter        __attribute__((aligned(8)));
	int     state;
	int     expire;
} __attribute__((aligned(8)));
#endif /* BSD_KERNEL_PRIVATE */

#define DRLSTSIZ 10
#define PRLSTSIZ 10

struct  in6_drlist {
	char ifname[IFNAMSIZ];
	struct {
		struct  in6_addr rtaddr;
		u_char  flags;
		u_short rtlifetime;
		u_long  expire;
		u_short if_index;
	} defrouter[DRLSTSIZ];
};

#if defined(BSD_KERNEL_PRIVATE)
struct  in6_drlist_32 {
	char ifname[IFNAMSIZ];
	struct {
		struct  in6_addr rtaddr;
		u_char  flags;
		u_short rtlifetime;
		u_int32_t expire;
		u_short if_index;
	} defrouter[DRLSTSIZ];
};

struct  in6_drlist_64 {
	char ifname[IFNAMSIZ];
	struct {
		struct  in6_addr rtaddr;
		u_char  flags;
		u_short rtlifetime;
		u_long  expire          __attribute__((aligned(8)));
		u_short if_index        __attribute__((aligned(8)));
	} defrouter[DRLSTSIZ] __attribute__((aligned(8)));
};
#endif /* BSD_KERNEL_PRIVATE */

/* valid values for stateflags */
#define NDDRF_INSTALLED 0x1     /* installed in the routing table */
#define NDDRF_IFSCOPE   0x2     /* installed as a scoped route */
#define NDDRF_STATIC    0x4     /* for internal use only */
#define NDDRF_MAPPED    0x8     /* Default router addr is mapped to a different one for routing */

struct  in6_defrouter {
	struct  sockaddr_in6 rtaddr;
	u_char  flags;
	u_char  stateflags;
	u_short rtlifetime;
	u_long  expire;
	u_short if_index;
};

#if defined(BSD_KERNEL_PRIVATE)
struct  in6_defrouter_32 {
	struct  sockaddr_in6 rtaddr;
	u_char  flags;
	u_char  stateflags;
	u_short rtlifetime;
	u_int32_t expire;
	u_short if_index;
};

struct  in6_defrouter_64 {
	struct  sockaddr_in6 rtaddr;
	u_char  flags;
	u_char  stateflags;
	u_short rtlifetime;
	u_long  expire          __attribute__((aligned(8)));
	u_short if_index        __attribute__((aligned(8)));
} __attribute__((aligned(8)));
#endif /* BSD_KERNEL_PRIVATE */

struct  in6_prlist {
	char ifname[IFNAMSIZ];
	struct {
		struct  in6_addr prefix;
		struct prf_ra raflags;
		u_char  prefixlen;
		u_char  origin;
		u_long  vltime;
		u_long  pltime;
		u_long  expire;
		u_short if_index;
		u_short advrtrs; /* number of advertisement routers */
		struct  in6_addr advrtr[DRLSTSIZ]; /* XXX: explicit limit */
	} prefix[PRLSTSIZ];
};

#if defined(BSD_KERNEL_PRIVATE)
struct  in6_prlist_32 {
	char ifname[IFNAMSIZ];
	struct {
		struct  in6_addr prefix;
		struct prf_ra raflags;
		u_char  prefixlen;
		u_char  origin;
		u_int32_t vltime;
		u_int32_t pltime;
		u_int32_t expire;
		u_short if_index;
		u_short advrtrs;
		struct  in6_addr advrtr[DRLSTSIZ];
	} prefix[PRLSTSIZ];
};

struct  in6_prlist_64 {
	char ifname[IFNAMSIZ];
	struct {
		struct  in6_addr prefix;
		struct prf_ra raflags;
		u_char  prefixlen;
		u_char  origin;
		u_long  vltime  __attribute__((aligned(8)));
		u_long  pltime  __attribute__((aligned(8)));
		u_long  expire  __attribute__((aligned(8)));
		u_short if_index;
		u_short advrtrs;
		u_int32_t pad;
		struct  in6_addr advrtr[DRLSTSIZ];
	} prefix[PRLSTSIZ];
};
#endif /* BSD_KERNEL_PRIVATE */

struct in6_prefix {
	struct  sockaddr_in6 prefix;
	struct prf_ra raflags;
	u_char  prefixlen;
	u_char  origin;
	u_long  vltime;
	u_long  pltime;
	u_long  expire;
	u_int32_t flags;
	int refcnt;
	u_short if_index;
	u_short advrtrs; /* number of advertisement routers */
	/* struct sockaddr_in6 advrtr[] */
};

#if defined(BSD_KERNEL_PRIVATE)
struct in6_prefix_32 {
	struct  sockaddr_in6 prefix;
	struct prf_ra raflags;
	u_char  prefixlen;
	u_char  origin;
	u_int32_t vltime;
	u_int32_t pltime;
	u_int32_t expire;
	u_int32_t flags;
	int refcnt;
	u_short if_index;
	u_short advrtrs; /* number of advertisement routers */
	/* struct sockaddr_in6 advrtr[] */
};

struct in6_prefix_64 {
	struct  sockaddr_in6 prefix;
	struct prf_ra raflags;
	u_char  prefixlen;
	u_char  origin;
	u_long  vltime  __attribute__((aligned(8)));
	u_long  pltime  __attribute__((aligned(8)));
	u_long  expire  __attribute__((aligned(8)));
	u_int32_t flags __attribute__((aligned(8)));
	int refcnt;
	u_short if_index;
	u_short advrtrs;
	/* struct sockaddr_in6 advrtr[] */
};
#endif /* BSD_KERNEL_PRIVATE */

struct  in6_ondireq {
	char ifname[IFNAMSIZ];
	struct {
		u_int32_t linkmtu;      /* LinkMTU */
		u_int32_t maxmtu;       /* Upper bound of LinkMTU */
		u_int32_t basereachable; /* BaseReachableTime */
		u_int32_t reachable;    /* Reachable Time */
		u_int32_t retrans;      /* Retrans Timer */
		u_int32_t flags;        /* Flags */
		int recalctm;           /* BaseReacable re-calculation timer */
		u_int8_t chlim;         /* CurHopLimit */
		u_int8_t receivedra;
	} ndi;
};

#if !defined(BSD_KERNEL_PRIVATE)
struct  in6_ndireq {
	char ifname[IFNAMSIZ];
	struct nd_ifinfo ndi;
};
#else
struct  in6_ndireq {
	char ifname[IFNAMSIZ];
	struct nd_ifinfo_compat ndi;
};
#endif /* !BSD_KERNEL_PRIVATE */

struct  in6_ndifreq {
	char ifname[IFNAMSIZ];
	u_long ifindex;
};

#define MAX_RTR_SOLICITATION_DELAY      1       /* 1sec */
#define RTR_SOLICITATION_INTERVAL       4       /* 4sec */

#if defined(BSD_KERNEL_PRIVATE)
struct  in6_ndifreq_32 {
	char ifname[IFNAMSIZ];
	u_int32_t ifindex;
};

struct  in6_ndifreq_64 {
	char ifname[IFNAMSIZ];
	u_int64_t ifindex  __attribute__((aligned(8)));
};
#endif /* BSD_KERNEL_PRIVATE */

/* Prefix status */
#define NDPRF_ONLINK            0x1
#define NDPRF_DETACHED          0x2
#define NDPRF_STATIC            0x100
#define NDPRF_IFSCOPE           0x1000
#define NDPRF_PRPROXY           0x2000
#ifdef BSD_KERNEL_PRIVATE
#define NDPRF_PROCESSED_ONLINK  0x08000
#define NDPRF_PROCESSED_SERVICE 0x10000
#define NDPRF_DEFUNCT           0x20000
#define NDPRF_CLAT46            0x40000
#endif

/* protocol constants */
#define MAX_RTR_SOLICITATION_DELAY      1       /* 1sec */
#define RTR_SOLICITATION_INTERVAL       4       /* 4sec */
#define MAX_RTR_SOLICITATIONS           3

#define ND6_INFINITE_LIFETIME           0xffffffff
#define ND6_MAX_LIFETIME                0x7fffffff

#ifdef BSD_KERNEL_PRIVATE
#define ND_IFINFO(ifp)                          \
    ((ifp == NULL) ? NULL :                     \
     ((IN6_IFEXTRA(ifp) == NULL) ? NULL :       \
      (&IN6_IFEXTRA(ifp)->nd_ifinfo)))

/*
 * In a more readable form, we derive linkmtu based on:
 *
 * if (ifp == NULL)
 *         linkmtu = IPV6_MMTU
 * else if (ND_IFINFO(ifp) == NULL || !ND_IFINFO(ifp)->initialized)
 *         linkmtu = ifp->if_mtu;
 * else if (ND_IFINFO(ifp)->linkmtu && ND_IFINFO(ifp)->linkmtu < ifp->if_mtu)
 *         linkmtu = ND_IFINFO(ifp)->linkmtu;
 * else if ((ND_IFINFO(ifp)->maxmtu && ND_IFINFO(ifp)->maxmtu < ifp->if_mtu))
 *         linkmtu = ND_IFINFO(ifp)->maxmtu;
 * else
 *         linkmtu = ifp->if_mtu;
 */
#define IN6_LINKMTU(ifp)                                                      \
	(ifp == NULL ? IPV6_MMTU :                                            \
	(ND_IFINFO(ifp) == NULL || !ND_IFINFO(ifp)->initialized) ?            \
	(ifp)->if_mtu :	((ND_IFINFO(ifp)->linkmtu &&                          \
	ND_IFINFO(ifp)->linkmtu < (ifp)->if_mtu) ? ND_IFINFO(ifp)->linkmtu :  \
	((ND_IFINFO(ifp)->maxmtu && ND_IFINFO(ifp)->maxmtu < (ifp)->if_mtu) ? \
	ND_IFINFO(ifp)->maxmtu : (ifp)->if_mtu)))

/* node constants */
#define MAX_REACHABLE_TIME              3600000 /* msec */
#define REACHABLE_TIME                  30000   /* msec */
#define RETRANS_TIMER                   1000    /* msec */
#define MIN_RANDOM_FACTOR               512     /* 1024 * 0.5 */
#define MAX_RANDOM_FACTOR               1536    /* 1024 * 1.5 */
#define DEF_TEMP_VALID_LIFETIME         604800  /* 1 week */
#define DEF_TEMP_PREFERRED_LIFETIME     86400   /* 1 day */
#define TEMPADDR_REGEN_ADVANCE          5       /* sec */
#define MAX_TEMP_DESYNC_FACTOR          600     /* 10 min */
#define ND_COMPUTE_RTIME(x) \
	        (((MIN_RANDOM_FACTOR * (x >> 10)) + (RandomULong() & \
	        ((MAX_RANDOM_FACTOR - MIN_RANDOM_FACTOR) * (x >> 10)))) /1000)

/* prefix expiry times */
#define ND6_PREFIX_EXPIRY_UNSPEC        -1
#define ND6_PREFIX_EXPIRY_NEVER         0

TAILQ_HEAD(nd_drhead, nd_defrouter);
struct  nd_defrouter {
	decl_lck_mtx_data(, nddr_lock);
	TAILQ_ENTRY(nd_defrouter) dr_entry;
	struct in6_addr rtaddr;
	u_int32_t       nddr_refcount;
	u_int32_t       nddr_debug;
	u_int64_t       expire;
	u_int64_t       base_calendartime;      /* calendar time at creation */
	u_int64_t       base_uptime;            /* uptime at creation */
	u_char          flags;                  /* flags on RA message */
	u_char          stateflags;
	u_short         rtlifetime;
	int             err;
	struct ifnet    *ifp;
	struct in6_addr rtaddr_mapped;          /* Mapped gateway address for routing */
	void (*nddr_trace)(struct nd_defrouter *, int); /* trace callback fn */
};

#define NDDR_LOCK_ASSERT_HELD(_nddr)                                    \
	LCK_MTX_ASSERT(&(_nddr)->nddr_lock, LCK_MTX_ASSERT_OWNED)

#define NDDR_LOCK_ASSERT_NOTHELD(_nddr)                                 \
	LCK_MTX_ASSERT(&(_nddr)->nddr_lock, LCK_MTX_ASSERT_NOTOWNED)

#define NDDR_LOCK(_nddr)                                                \
	lck_mtx_lock(&(_nddr)->nddr_lock)

#define NDDR_LOCK_SPIN(_nddr)                                           \
	lck_mtx_lock_spin(&(_nddr)->nddr_lock)

#define NDDR_CONVERT_LOCK(_nddr) do {                                   \
	NDPR_LOCK_ASSERT_HELD(_nddr);                                   \
	lck_mtx_convert_spin(&(_nddr)->nddr_lock);                      \
} while (0)

#define NDDR_UNLOCK(_nddr)                                              \
	lck_mtx_unlock(&(_nddr)->nddr_lock)

#define NDDR_ADDREF(_nddr)                                              \
	nddr_addref(_nddr, 0)

#define NDDR_ADDREF_LOCKED(_nddr)                                       \
	nddr_addref(_nddr, 1)

#define NDDR_REMREF(_nddr) do {                                         \
	(void) nddr_remref(_nddr, 0);                                   \
} while (0)

#define NDDR_REMREF_LOCKED(_nddr)                                       \
	nddr_remref(_nddr, 1)

/* define struct prproxy_sols_tree */
RB_HEAD(prproxy_sols_tree, nd6_prproxy_soltgt);

struct nd_prefix {
	decl_lck_mtx_data(, ndpr_lock);
	u_int32_t       ndpr_refcount;  /* reference count */
	u_int32_t       ndpr_debug;     /* see ifa_debug flags */
	struct ifnet     *ndpr_ifp;
	struct rtentry   *ndpr_rt;
	LIST_ENTRY(nd_prefix) ndpr_entry;
	struct sockaddr_in6 ndpr_prefix; /* prefix */
	struct in6_addr ndpr_mask; /* netmask derived from the prefix */
	struct in6_addr ndpr_addr; /* address that is derived from the prefix */
	u_int32_t       ndpr_vltime;    /* advertised valid lifetime */
	u_int32_t       ndpr_pltime;    /* advertised preferred lifetime */
	u_int64_t       ndpr_preferred; /* preferred time of the prefix */
	u_int64_t       ndpr_expire;    /* expiration time of the prefix */
	u_int64_t       ndpr_lastupdate; /* rx time of last advertisement */
	u_int64_t       ndpr_base_calendartime; /* calendar time at creation */
	u_int64_t       ndpr_base_uptime;       /* uptime at creation */
	struct prf_ra   ndpr_flags;
	unsigned int    ndpr_genid;     /* protects ndpr_advrtrs */
	u_int32_t       ndpr_stateflags; /* actual state flags */
	/* list of routers that advertise the prefix: */
	LIST_HEAD(pr_rtrhead, nd_pfxrouter) ndpr_advrtrs;
	u_char          ndpr_plen;
	int             ndpr_addrcnt;   /* reference counter from addresses */
	u_int32_t       ndpr_allmulti_cnt;      /* total all-multi reqs */
	u_int32_t       ndpr_prproxy_sols_cnt;  /* total # of proxied NS */
	struct prproxy_sols_tree ndpr_prproxy_sols; /* tree of proxied NS */
	void (*ndpr_trace)(struct nd_prefix *, int); /* trace callback fn */
};

#define ndpr_next               ndpr_entry.le_next

#define ndpr_raf                ndpr_flags
#define ndpr_raf_onlink         ndpr_flags.onlink
#define ndpr_raf_auto           ndpr_flags.autonomous
#define ndpr_raf_router         ndpr_flags.router
/*
 * We keep expired prefix for certain amount of time, for validation purposes.
 * 1800s = MaxRtrAdvInterval
 */
#define NDPR_KEEP_EXPIRED       (1800 * 2)

#define NDPR_LOCK_ASSERT_HELD(_ndpr)                                    \
	LCK_MTX_ASSERT(&(_ndpr)->ndpr_lock, LCK_MTX_ASSERT_OWNED)

#define NDPR_LOCK_ASSERT_NOTHELD(_ndpr)                                 \
	LCK_MTX_ASSERT(&(_ndpr)->ndpr_lock, LCK_MTX_ASSERT_NOTOWNED)

#define NDPR_LOCK(_ndpr)                                                \
	lck_mtx_lock(&(_ndpr)->ndpr_lock)

#define NDPR_LOCK_SPIN(_ndpr)                                           \
	lck_mtx_lock_spin(&(_ndpr)->ndpr_lock)

#define NDPR_CONVERT_LOCK(_ndpr) do {                                   \
	NDPR_LOCK_ASSERT_HELD(_ndpr);                                   \
	lck_mtx_convert_spin(&(_ndpr)->ndpr_lock);                      \
} while (0)

#define NDPR_UNLOCK(_ndpr)                                              \
	lck_mtx_unlock(&(_ndpr)->ndpr_lock)

#define NDPR_ADDREF(_ndpr)                                              \
	ndpr_addref(_ndpr, 0)

#define NDPR_ADDREF_LOCKED(_ndpr)                                       \
	ndpr_addref(_ndpr, 1)

#define NDPR_REMREF(_ndpr) do {                                         \
	(void) ndpr_remref(_ndpr, 0);                                   \
} while (0)

#define NDPR_REMREF_LOCKED(_ndpr)                                       \
	ndpr_remref(_ndpr, 1)

/*
 * Message format for use in obtaining information about prefixes
 * from inet6 sysctl function
 */
struct inet6_ndpr_msghdr {
	u_short inpm_msglen;    /* to skip over non-understood messages */
	u_char  inpm_version;   /* future binary compatibility */
	u_char  inpm_type;      /* message type */
	struct in6_addr inpm_prefix;
	u_int32_t       prm_vltim;
	u_int32_t       prm_pltime;
	u_int32_t       prm_expire;
	u_int32_t       prm_preferred;
	struct in6_prflags prm_flags;
	u_short prm_index;      /* index for associated ifp */
	u_char  prm_plen;       /* length of prefix in bits */
};

#define prm_raf_onlink          prm_flags.prf_ra.onlink
#define prm_raf_auto            prm_flags.prf_ra.autonomous

#define prm_statef_onlink       prm_flags.prf_state.onlink

#define prm_rrf_decrvalid       prm_flags.prf_rr.decrvalid
#define prm_rrf_decrprefd       prm_flags.prf_rr.decrprefd

struct nd_pfxrouter {
	LIST_ENTRY(nd_pfxrouter) pfr_entry;
#define pfr_next pfr_entry.le_next
	struct nd_defrouter *router;
};

LIST_HEAD(nd_prhead, nd_prefix);

struct nd_prefix_list {
	struct nd_prefix_list *next;
	struct nd_prefix pr;
};
#endif /* BSD_KERNEL_PRIVATE */

#if defined(PRIVATE)
struct kev_nd6_ndfailure {
	struct net_event_data link_data;
};

struct kev_nd6_ndalive {
	struct net_event_data link_data;
};

struct nd6_ra_prefix {
	struct sockaddr_in6 prefix;
	struct prf_ra raflags;
	u_int32_t prefixlen;
	u_int32_t origin;
	u_int64_t vltime;
	u_int64_t pltime;
	u_int64_t expire;
	u_int32_t flags;
	u_int32_t refcnt;
	u_int32_t if_index;
	u_int32_t pad;
};

/* ND6 router advertisement valid bits */
#define KEV_ND6_DATA_VALID_MTU          (0x1 << 0)
#define KEV_ND6_DATA_VALID_PREFIX       (0x1 << 1)

struct kev_nd6_ra_data {
	u_int32_t mtu;
	u_int32_t list_index;
	u_int32_t list_length;
	u_int32_t flags;
	struct nd6_ra_prefix prefix;
	u_int32_t pad;
};

struct kev_nd6_event {
	struct net_event_data link_data;
	struct in6_addr in6_address;
	uint32_t val;
};

struct nd6_lookup_ipv6_args {
	char ifname[IFNAMSIZ];
	struct sockaddr_in6 ip6_dest;
	u_int32_t ll_dest_len;
	union {
		char buffer[256];
		struct sockaddr_dl _sdl;
	} ll_dest_;
};
#define ll_dest_sdl ll_dest_._sdl

#endif /* PRIVATE */

#if defined(BSD_KERNEL_PRIVATE)
/* nd6.c */
extern int nd6_prune;
extern int nd6_prune_lazy;
extern int nd6_delay;
extern int nd6_umaxtries;
extern int nd6_mmaxtries;
extern int nd6_useloopback;
extern int nd6_accept_6to4;
extern int nd6_maxnudhint;
extern int nd6_gctimer;
extern struct llinfo_nd6 llinfo_nd6;
extern struct nd_drhead nd_defrouter;
extern struct nd_prhead nd_prefix;
extern int nd6_debug;
extern int nd6_onlink_ns_rfc4861;
extern int nd6_optimistic_dad;

#include <os/log.h>

#define nd6log0(type, ...)      do { os_log_##type(OS_LOG_DEFAULT, __VA_ARGS__); } while (0)
#define nd6log(type, ...)       do { if (nd6_debug >= 1) os_log_##type(OS_LOG_DEFAULT, __VA_ARGS__); } while (0)
#define nd6log2(type, ...)      do { if (nd6_debug >= 2) os_log_##type(OS_LOG_DEFAULT, __VA_ARGS__); } while (0)

#define ND6_OPTIMISTIC_DAD_LINKLOCAL    (1 << 0)
#define ND6_OPTIMISTIC_DAD_AUTOCONF     (1 << 1)
#define ND6_OPTIMISTIC_DAD_TEMPORARY    (1 << 2)
#define ND6_OPTIMISTIC_DAD_DYNAMIC      (1 << 3)
#define ND6_OPTIMISTIC_DAD_SECURED      (1 << 4)
#define ND6_OPTIMISTIC_DAD_MANUAL       (1 << 5)

/* nd6_rtr.c */
extern int nd6_defifindex;
extern int ip6_desync_factor;   /* seconds */
/* ND6_INFINITE_LIFETIME does not apply to temporary addresses */
extern u_int32_t ip6_temp_preferred_lifetime; /* seconds */
extern u_int32_t ip6_temp_valid_lifetime; /* seconds */
extern int ip6_temp_regen_advance; /* seconds */

union nd_opts {
	struct nd_opt_hdr *nd_opt_array[16];    /* max = target address list */
	struct {
		struct nd_opt_hdr *zero;
		struct nd_opt_hdr *src_lladdr;
		struct nd_opt_hdr *tgt_lladdr;
		struct nd_opt_prefix_info *pi_beg; /* multiple opts, start */
		struct nd_opt_rd_hdr *rh;
		struct nd_opt_mtu *mtu;
		struct nd_opt_hdr *__res6;
		struct nd_opt_hdr *__res7;
		struct nd_opt_hdr *__res8;
		struct nd_opt_hdr *__res9;
		struct nd_opt_hdr *__res10;
		struct nd_opt_hdr *__res11;
		struct nd_opt_hdr *__res12;
		struct nd_opt_hdr *__res13;
		struct nd_opt_nonce *nonce;
		struct nd_opt_hdr *__res15;
		struct nd_opt_hdr *search;      /* multiple opts */
		struct nd_opt_hdr *last;        /* multiple opts */
		int done;
		struct nd_opt_prefix_info *pi_end; /* multiple opts, end */
	} nd_opt_each;
};
#define nd_opts_src_lladdr      nd_opt_each.src_lladdr
#define nd_opts_tgt_lladdr      nd_opt_each.tgt_lladdr
#define nd_opts_pi              nd_opt_each.pi_beg
#define nd_opts_pi_end          nd_opt_each.pi_end
#define nd_opts_rh              nd_opt_each.rh
#define nd_opts_mtu             nd_opt_each.mtu
#define nd_opts_nonce           nd_opt_each.nonce
#define nd_opts_search          nd_opt_each.search
#define nd_opts_last            nd_opt_each.last
#define nd_opts_done            nd_opt_each.done

/* XXX: need nd6_var.h?? */
/* nd6.c */
extern int nd6_sched_timeout_want;
extern void nd6_sched_timeout(struct timeval *, struct timeval *);
extern void nd6_init(void);
extern void nd6_ifreset(struct ifnet *ifp);
extern void nd6_ifattach(struct ifnet *);
extern int nd6_is_addr_neighbor(struct sockaddr_in6 *, struct ifnet *, int);
extern void nd6_option_init(void *, int, union nd_opts *);
extern struct nd_opt_hdr *nd6_option(union nd_opts *);
extern int nd6_options(union nd_opts *);
extern struct rtentry *nd6_lookup(struct in6_addr *, int, struct ifnet *, int);
extern void nd6_setmtu(struct ifnet *);
extern void nd6_purge(struct ifnet *);
extern void nd6_free(struct rtentry *);
extern void nd6_nud_hint(struct rtentry *, struct in6_addr *, int);
extern int nd6_resolve(struct ifnet *, struct rtentry *,
    struct mbuf *, struct sockaddr *, u_char *);
extern void nd6_rtrequest(int, struct rtentry *, struct sockaddr *);
extern int nd6_ioctl(u_long, caddr_t, struct ifnet *);
extern void nd6_cache_lladdr(struct ifnet *, struct in6_addr *,
    char *, int, int, int);
extern int nd6_output_list(struct ifnet *, struct ifnet *, struct mbuf *,
    struct sockaddr_in6 *, struct rtentry *, struct flowadv *);
extern int nd6_output(struct ifnet *, struct ifnet *, struct mbuf *,
    struct sockaddr_in6 *, struct rtentry *, struct flowadv *);
extern int nd6_storelladdr(struct ifnet *, struct rtentry *, struct mbuf *,
    struct sockaddr *, u_char *);
extern int nd6_need_cache(struct ifnet *);
extern void nd6_drain(void *);
extern void nd6_post_msg(u_int32_t, struct nd_prefix_list *, u_int32_t,
    u_int32_t);
extern int nd6_setifinfo(struct ifnet *, u_int32_t, u_int32_t);
extern const char *ndcache_state2str(short);
extern void ln_setexpire(struct llinfo_nd6 *, uint64_t);

/* nd6_nbr.c */
extern void nd6_nbr_init(void);
extern void nd6_na_input(struct mbuf *, int, int);
extern void nd6_na_output(struct ifnet *, const struct in6_addr *,
    const struct in6_addr *, u_int32_t, int, struct sockaddr *);
extern void nd6_ns_input(struct mbuf *, int, int);
extern void nd6_ns_output(struct ifnet *, const struct in6_addr *,
    const struct in6_addr *, struct llinfo_nd6 *, uint8_t *);
extern caddr_t nd6_ifptomac(struct ifnet *);
extern void nd6_dad_start(struct ifaddr *, int *);
extern void nd6_dad_stop(struct ifaddr *);
extern void nd6_dad_duplicated(struct ifaddr *);
extern void nd6_llreach_alloc(struct rtentry *, struct ifnet *, void *,
    unsigned int, boolean_t);
extern void nd6_llreach_set_reachable(struct ifnet *, void *, unsigned int);
extern void nd6_llreach_use(struct llinfo_nd6 *);
extern void nd6_alt_node_addr_decompose(struct ifnet *, struct sockaddr *,
    struct sockaddr_dl *, struct sockaddr_in6 *);
extern int nd6_alt_node_present(struct ifnet *, struct sockaddr_in6 *,
    struct sockaddr_dl *, int32_t, int, int);
extern void nd6_alt_node_absent(struct ifnet *, struct sockaddr_in6 *, struct sockaddr_dl *);

/* nd6_rtr.c */
extern struct in6_ifaddr *in6_pfx_newpersistaddr(struct nd_prefix *, int,
    int *, boolean_t);
extern void nd6_rtr_init(void);
extern void nd6_rs_input(struct mbuf *, int, int);
extern void nd6_ra_input(struct mbuf *, int, int);
extern void prelist_del(struct nd_prefix *);
extern void defrouter_select(struct ifnet *);
extern void defrouter_reset(void);
extern int defrtrlist_ioctl(u_long, caddr_t);
extern void defrtrlist_del(struct nd_defrouter *);
extern int defrtrlist_add_static(struct nd_defrouter *);
extern int defrtrlist_del_static(struct nd_defrouter *);
extern void prelist_remove(struct nd_prefix *);
extern int prelist_update(struct nd_prefix *, struct nd_defrouter *,
    struct mbuf *, int);
extern int nd6_prelist_add(struct nd_prefix *, struct nd_defrouter *,
    struct nd_prefix **, boolean_t);
extern int nd6_prefix_onlink(struct nd_prefix *);
extern int nd6_prefix_onlink_scoped(struct nd_prefix *, unsigned int);
extern int nd6_prefix_offlink(struct nd_prefix *);
extern void pfxlist_onlink_check(void);
extern struct nd_defrouter *defrouter_lookup(struct in6_addr *, struct ifnet *);
extern struct nd_prefix *nd6_prefix_lookup(struct nd_prefix *, int);
extern int in6_init_prefix_ltimes(struct nd_prefix *ndpr);
extern void rt6_flush(struct in6_addr *, struct ifnet *);
extern int nd6_setdefaultiface(int);
extern int in6_tmpifadd(const struct in6_ifaddr *, int);
extern void nddr_addref(struct nd_defrouter *, int);
extern struct nd_defrouter *nddr_remref(struct nd_defrouter *, int);
extern uint64_t nddr_getexpire(struct nd_defrouter *);
extern void ndpr_addref(struct nd_prefix *, int);
extern struct nd_prefix *ndpr_remref(struct nd_prefix *, int);
extern uint64_t ndpr_getexpire(struct nd_prefix *);

/* nd6_prproxy.c */
struct ip6_hdr;
extern u_int32_t nd6_prproxy;
extern void nd6_prproxy_init(void);
extern int nd6_if_prproxy(struct ifnet *, boolean_t);
extern void nd6_prproxy_prelist_update(struct nd_prefix *, struct nd_prefix *);
extern boolean_t nd6_prproxy_ifaddr(struct in6_ifaddr *);
extern void nd6_proxy_find_fwdroute(struct ifnet *, struct route_in6 *);
extern boolean_t nd6_prproxy_isours(struct mbuf *, struct ip6_hdr *,
    struct route_in6 *, unsigned int);
extern void nd6_prproxy_ns_output(struct ifnet *, struct ifnet *,
    struct in6_addr *, struct in6_addr *, struct llinfo_nd6 *);
extern void nd6_prproxy_ns_input(struct ifnet *, struct in6_addr *,
    char *, int, struct in6_addr *, struct in6_addr *, uint8_t *nonce);
extern void nd6_prproxy_na_input(struct ifnet *, struct in6_addr *,
    struct in6_addr *, struct in6_addr *, int);
extern void nd6_prproxy_sols_reap(struct nd_prefix *);
extern void nd6_prproxy_sols_prune(struct nd_prefix *, u_int32_t);
extern int nd6_if_disable(struct ifnet *, boolean_t);
void in6_ifaddr_set_dadprogress(struct in6_ifaddr *ia);
#endif /* BSD_KERNEL_PRIVATE */

#ifdef KERNEL

/*
 *	@function nd6_lookup_ipv6
 *	@discussion This function will check the routing table for a cached
 *		neighbor discovery entry or trigger an neighbor discovery query
 *		to resolve the IPv6 address to a link-layer address.
 *		nd entries are stored in the routing table. This function will
 *		lookup the IPv6 destination in the routing table. If the
 *		destination requires forwarding to a gateway, the route of the
 *		gateway will be looked up. The route entry is inspected to
 *		determine if the link layer destination address is known. If
 *		unknown, neighbor discovery will be used to resolve the entry.
 *	@param interface The interface the packet is being sent on.
 *	@param ip6_dest The IPv6 destination of the packet.
 *	@param ll_dest On output, the link-layer destination.
 *	@param ll_dest_len The length of the buffer for ll_dest.
 *	@param hint Any routing hint passed down from the protocol.
 *	@param packet The packet being transmitted.
 *	@result May return an error such as EHOSTDOWN or ENETUNREACH. If
 *		this function returns EJUSTRETURN, the packet has been queued
 *		and will be sent when the address is resolved. If any other
 *		value is returned, the caller is responsible for disposing of
 *		the packet.
 */
extern errno_t nd6_lookup_ipv6(ifnet_t interface,
    const struct sockaddr_in6 *ip6_dest, struct sockaddr_dl *ll_dest,
    size_t ll_dest_len, route_t hint, mbuf_t packet);

#endif /* KERNEL */

/* nd6_send.c */
#ifdef BSD_KERNEL_PRIVATE
/*
 * nd6_send_opmode
 *
 *      value	using CGA	tx SEND		rx SEND
 *   --------	---------	-------		-------
 *   DISABLED	       NO	     NO		     NO
 *      QUIET	      YES	     NO		     NO
 */
extern int nd6_send_opstate;

#define ND6_SEND_OPMODE_DISABLED        0
#define ND6_SEND_OPMODE_CGA_QUIET       1

#endif /* BSD_KERNEL_PRIVATE */
#endif /* _NETINET6_ND6_H_ */
