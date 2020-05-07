/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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
 * Copyright (c) 2004 Luigi Rizzo, Alessandro Cerri. All rights reserved.
 * Copyright (c) 2004-2008 Qing Li. All rights reserved.
 * Copyright (c) 2008 Kip Macy. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <sys/cdefs.h>

#ifndef _NET_IF_LLATBL_H_
#define _NET_IF_LLATBL_H_

#include <netinet/in.h>
#include <net/if_dl.h>
#include <kern/locks.h>
#include <kern/thread_call.h>
#include <sys/eventhandler.h>

struct ifnet;
struct sysctl_req;
struct rt_msghdr;
struct rt_addrinfo;

struct llentry;
LIST_HEAD(llentries, llentry);

extern lck_rw_t                *lltable_rwlock;
#define LLTABLE_RLOCK()         lck_rw_lock_shared(lltable_rwlock)
#define LLTABLE_RUNLOCK()       lck_rw_done(lltable_rwlock)
#define LLTABLE_WLOCK()         lck_rw_lock_exclusive(lltable_rwlock)
#define LLTABLE_WUNLOCK()       lck_rw_done(lltable_rwlock)
#define LLTABLE_LOCK_ASSERT()   LCK_RW_ASSERT(lltable_rwlock, LCK_RW_ASSERT_EXCLUSIVE)

#define LLE_MAX_LINKHDR         24      /* Full IB header */
/*
 * Code referencing llentry must at least hold
 * a shared lock
 */
struct llentry {
	LIST_ENTRY(llentry)      lle_next;
	union {
		struct in_addr  addr4;
		struct in6_addr addr6;
	} r_l3addr;
#if 0
	char                    r_linkdata[LLE_MAX_LINKHDR]; /* L2 data */
	uint8_t                 r_hdrlen;       /* length for LL header */
	uint8_t                 spare0[3];
#endif
	uint16_t                r_flags;        /* LLE runtime flags */
	uint16_t                r_skip_req;     /* feedback from fast path */

	struct lltable           *lle_tbl;
	struct llentries         *lle_head;
	void                    (*lle_free)(struct llentry *);
	struct mbuf              *la_hold;
	int                      la_numheld;  /* # of packets currently held */
	u_int64_t                        la_expire;
	uint16_t                 la_flags;
	uint16_t                 la_asked;
	uint16_t                 la_preempt;
	int16_t                  ln_state;      /* IPv6 has ND6_LLINFO_NOSTATE == -2 */
	uint16_t                 ln_router;
	time_t                   ln_ntick;
	time_t                  lle_remtime;    /* Real time remaining */
	time_t                  lle_hittime;    /* Time when r_skip_req was unset */
	int                      lle_refcnt;
	union {
		uint64_t        mac_aligned;
		uint16_t        mac16[3];
	} ll_addr;
	LIST_ENTRY(llentry)     lle_chain;      /* chain of deleted items */
	thread_call_t           lle_timer;
	u_int64_t ln_lastused;  /* last used timestamp */
	struct  if_llreach *ln_llreach; /* link-layer reachability record */
	decl_lck_rw_data(, lle_lock);
	decl_lck_mtx_data(, req_mtx);
};

extern lck_grp_t      *lle_lock_grp;
extern lck_attr_t     *lle_lock_attr;

#define LLE_WLOCK(lle)          lck_rw_lock_exclusive(&(lle)->lle_lock)
#define LLE_RLOCK(lle)          lck_rw_lock_shared(&(lle)->lle_lock)
#define LLE_WUNLOCK(lle)        lck_rw_done(&(lle)->lle_lock)
#define LLE_RUNLOCK(lle)        lck_rw_done(&(lle)->lle_lock)
#define LLE_DOWNGRADE(lle)      lck_rw_lock_exclusive_to_shared(&(lle)->lle_lock)
#define LLE_TRY_UPGRADE(lle)    lck_rw_lock_shared_to_exclusive(&(lle)->lle_lock)
#define LLE_LOCK_INIT(lle)      lck_rw_init(&(lle)->lle_lock, lle_lock_grp, lle_lock_attr)
#define LLE_LOCK_DESTROY(lle)   lck_rw_destroy(&(lle)->lle_lock, lle_lock_grp)
#define LLE_WLOCK_ASSERT(lle)   LCK_RW_ASSERT(&(lle)->lle_lock, LCK_RW_ASSERT_EXCLUSIVE)

#define LLE_REQ_INIT(lle)       lck_mtx_init(&(lle)->req_mtx, lle_lock_grp, lle_lock_attr)
#define LLE_REQ_DESTROY(lle)    lck_mtx_destroy(&(lle)->req_mtx, lle_lock_grp)
#define LLE_REQ_LOCK(lle)       lck_mtx_lock(&(lle)->req_mtx)
#define LLE_REQ_UNLOCK(lle)     lck_mtx_unlock(&(lle)->req_mtx)

#define LLE_IS_VALID(lle)       (((lle) != NULL) && ((lle) != (void *)-1))

#define LLE_ADDREF(lle) do {                                    \
	LLE_WLOCK_ASSERT(lle);                                  \
	VERIFY((lle)->lle_refcnt >= 0);                         \
	(lle)->lle_refcnt++;                                    \
} while (0)

#define LLE_REMREF(lle) do {                                    \
	LLE_WLOCK_ASSERT(lle);                                  \
	VERIFY((lle)->lle_refcnt > 0);                          \
	(lle)->lle_refcnt--;                                    \
} while (0)

#define LLE_FREE_LOCKED(lle) do {                               \
	if ((lle)->lle_refcnt == 1)                             \
	        (lle)->lle_free(lle);                           \
	else {                                                  \
	        LLE_REMREF(lle);                                \
	        LLE_WUNLOCK(lle);                               \
	}                                                       \
	/* guard against invalid refs */                        \
	(lle) = NULL;                                           \
} while (0)

#define LLE_FREE(lle) do {                                      \
	LLE_WLOCK(lle);                                         \
	LLE_FREE_LOCKED(lle);                                   \
} while (0)

typedef struct llentry *(llt_lookup_t)(struct lltable *, u_int flags,
    const struct sockaddr *l3addr);
typedef struct llentry *(llt_alloc_t)(struct lltable *, u_int flags,
    const struct sockaddr *l3addr);
typedef void (llt_delete_t)(struct lltable *, struct llentry *);
typedef void (llt_prefix_free_t)(struct lltable *,
    const struct sockaddr *addr, const struct sockaddr *mask, u_int flags);
typedef int (llt_dump_entry_t)(struct lltable *, struct llentry *,
    struct sysctl_req *);
typedef uint32_t (llt_hash_t)(const struct llentry *, uint32_t);
typedef int (llt_match_prefix_t)(const struct sockaddr *,
    const struct sockaddr *, u_int, struct llentry *);
typedef void (llt_free_entry_t)(struct lltable *, struct llentry *);
typedef void (llt_fill_sa_entry_t)(const struct llentry *, struct sockaddr *);
typedef void (llt_free_tbl_t)(struct lltable *);
typedef void (llt_link_entry_t)(struct lltable *, struct llentry *);
typedef void (llt_unlink_entry_t)(struct llentry *);

typedef int (llt_foreach_cb_t)(struct lltable *, struct llentry *, void *);
typedef int (llt_foreach_entry_t)(struct lltable *, llt_foreach_cb_t *, void *);

struct lltable {
	SLIST_ENTRY(lltable)    llt_link;
	int                     llt_af;
	int                     llt_hsize;
	struct llentries        *lle_head;
	struct ifnet            *llt_ifp;

	llt_lookup_t            *llt_lookup;
	llt_alloc_t             *llt_alloc_entry;
	llt_delete_t            *llt_delete_entry;
	llt_prefix_free_t       *llt_prefix_free;
	llt_dump_entry_t        *llt_dump_entry;
	llt_hash_t              *llt_hash;
	llt_match_prefix_t      *llt_match_prefix;
	llt_free_entry_t        *llt_free_entry;
	llt_foreach_entry_t     *llt_foreach_entry;
	llt_link_entry_t        *llt_link_entry;
	llt_unlink_entry_t      *llt_unlink_entry;
	llt_fill_sa_entry_t     *llt_fill_sa_entry;
	llt_free_tbl_t          *llt_free_tbl;
};

#ifdef MALLOC_DECLARE
MALLOC_DECLARE(M_LLTABLE);
#endif

/*
 * LLentry flags
 */
#define LLE_DELETED     0x0001  /* entry must be deleted */
#define LLE_STATIC      0x0002  /* entry is static */
#define LLE_IFADDR      0x0004  /* entry is interface addr */
#define LLE_VALID       0x0008  /* ll_addr is valid */
#define LLE_REDIRECT    0x0010  /* installed by redirect; has host rtentry */
#define LLE_PUB         0x0020  /* publish entry ??? */
#define LLE_LINKED      0x0040  /* linked to lookup structure */
/* LLE request flags */
#define LLE_EXCLUSIVE   0x2000  /* return lle xlocked  */
#define LLE_UNLOCKED    0x4000  /* return lle unlocked */
#define LLE_ADDRONLY    0x4000  /* return lladdr instead of full header */
#define LLE_CREATE      0x8000  /* hint to avoid lle lookup */

/* LLE flags used by fastpath code */
#define RLLE_VALID      0x0001          /* entry is valid */
#define RLLE_IFADDR     LLE_IFADDR      /* entry is ifaddr */

#define LLATBL_HASH(key, mask) \
    (((((((key >> 8) ^ key) >> 8) ^ key) >> 8) ^ key) & mask)

void lltable_glbl_init(void);
struct lltable *lltable_allocate_htbl(uint32_t hsize);
void lltable_free(struct lltable *);
void lltable_link(struct lltable *llt);
void lltable_prefix_free(int, struct sockaddr *,
    struct sockaddr *, u_int);
#if 0
void            lltable_drain(int);
#endif
int             lltable_sysctl_dumparp(int, struct sysctl_req *);

size_t          llentry_free(struct llentry *);
struct llentry  *llentry_alloc(struct ifnet *, struct lltable *,
    struct sockaddr_storage *);

/* helper functions */
size_t lltable_drop_entry_queue(struct llentry *);
void lltable_set_entry_addr(struct ifnet *ifp, struct llentry *lle,
    const char *ll_addr);
int lltable_try_set_entry_addr(struct ifnet *ifp, struct llentry *lle,
    const char *linkhdr, size_t linkhdrsize, int lladdr_off);

int lltable_calc_llheader(struct ifnet *ifp, int family, char *lladdr,
    char *buf, size_t *bufsize, int *lladdr_off);
void lltable_update_ifaddr(struct lltable *llt);
struct llentry *lltable_alloc_entry(struct lltable *llt, u_int flags,
    const struct sockaddr *l4addr);
void lltable_free_entry(struct lltable *llt, struct llentry *lle);
int lltable_delete_addr(struct lltable *llt, u_int flags,
    const struct sockaddr *l3addr);
void lltable_link_entry(struct lltable *llt, struct llentry *lle);
void lltable_unlink_entry(struct lltable *llt, struct llentry *lle);
void lltable_fill_sa_entry(const struct llentry *lle, struct sockaddr *sa);
struct ifnet *lltable_get_ifp(const struct lltable *llt);
int lltable_get_af(const struct lltable *llt);

int lltable_foreach_lle(struct lltable *llt, llt_foreach_cb_t *f,
    void *farg);
/*
 * Generic link layer address lookup function.
 */
static __inline struct llentry *
lla_lookup(struct lltable *llt, u_int flags, const struct sockaddr *l3addr)
{
	return llt->llt_lookup(llt, flags, l3addr);
}

int lla_rt_output(struct rt_msghdr *, struct rt_addrinfo *);

enum {
	LLENTRY_RESOLVED,
	LLENTRY_TIMEDOUT,
	LLENTRY_DELETED,
	LLENTRY_EXPIRED,
};

typedef void (*lle_event_fn)(struct eventhandler_entry_arg, struct llentry *, int);
EVENTHANDLER_DECLARE(lle_event, lle_event_fn);
#endif  /* _NET_IF_LLATBL_H_ */
