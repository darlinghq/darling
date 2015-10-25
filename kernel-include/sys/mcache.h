/*
 * Copyright (c) 2006-2012 Apple Inc. All rights reserved.
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
#ifndef _SYS_MCACHE_H
#define	_SYS_MCACHE_H

#ifdef KERNEL_PRIVATE

#ifdef  __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/queue.h>
#include <mach/boolean.h>
#include <kern/locks.h>
#include <libkern/OSAtomic.h>

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef VERIFY
#undef VERIFY
#endif

/*
 * Unlike VERIFY(), ASSERT() is evaluated only in DEBUG build.
 */
#define	VERIFY(EX)	((void)((EX) || assfail(#EX, __FILE__, __LINE__)))
#if DEBUG
#define	ASSERT(EX)	VERIFY(EX)
#else
#define	ASSERT(EX)	((void)0)
#endif

/*
 * Compile time assert; this should be on its own someday.
 */
#define	_CASSERT(x)	\
	switch (0) { case 0: case (x): ; }

/*
 * Atomic macros; these should be on their own someday.
 */
#define	atomic_add_16_ov(a, n)						\
	((u_int16_t) OSAddAtomic16(n, (volatile SInt16 *)a))

#define	atomic_add_16(a, n)						\
	((void) atomic_add_16_ov(a, n))

#define	atomic_add_32_ov(a, n)						\
	((u_int32_t) OSAddAtomic(n, (volatile SInt32 *)a))

#define	atomic_add_32(a, n)						\
	((void) atomic_add_32_ov(a, n))

#define	atomic_add_64_ov(a, n)						\
	((u_int64_t) OSAddAtomic64(n, (volatile SInt64 *)a))

#define	atomic_add_64(a, n)						\
	((void) atomic_add_64_ov(a, n))

#define	atomic_set_64(a, n) do {					\
	while (!OSCompareAndSwap64(*a, n, (volatile UInt64 *)a))	\
		;							\
} while (0)

#if defined(__LP64__)
#define	atomic_get_64(n, a) do {					\
	(n) = *(a);							\
} while (0)
#else
#define	atomic_get_64(n, a) do {					\
	(n) = atomic_add_64_ov(a, 0);					\
} while (0)
#endif /* __LP64__ */

#define	CPU_CACHE_SIZE	64

#ifndef IS_P2ALIGNED
#define	IS_P2ALIGNED(v, a) \
	((((uintptr_t)(v)) & ((uintptr_t)(a) - 1)) == 0)
#endif /* IS_P2ALIGNED */

#ifndef P2ROUNDUP
#define	P2ROUNDUP(x, align) \
	(-(-((uintptr_t)(x)) & -(align)))
#endif /* P2ROUNDUP */

#ifndef P2ROUNDDOWN
#define	P2ROUNDDOWN(x, align) \
	(((uintptr_t)(x)) & ~((uintptr_t)(align) - 1))
#endif /* P2ROUNDDOWN */

#define	MCACHE_FREE_PATTERN		0xdeadbeefdeadbeefULL
#define	MCACHE_UNINITIALIZED_PATTERN	0xbaddcafebaddcafeULL

/*
 * mcache allocation request flags.
 *
 * MCR_NOSLEEP and MCR_FAILOK are mutually exclusive.  The latter is used
 * by the mbuf allocator to handle the implementation of several caches that
 * involve multiple layers of mcache.  It implies a best effort blocking
 * allocation request; if the request cannot be satisfied, the caller will
 * be blocked until further notice, similar to MCR_SLEEP, except that upon
 * a wake up it will return immediately to the caller regardless of whether
 * the request can been fulfilled.
 *
 * MCR_TRYHARD implies a non-blocking allocation request, regardless of
 * whether MCR_NOSLEEP is set.  It informs the allocator that the request
 * should not cause the calling thread to block, and that it must have
 * exhausted all possible schemes to fulfill the request, including doing
 * reclaims and/or purges, before returning to the caller.
 *
 * Regular mcache clients should only use MCR_SLEEP or MCR_NOSLEEP.
 */
#define	MCR_SLEEP	0x0000		/* same as M_WAITOK */
#define	MCR_NOSLEEP	0x0001		/* same as M_NOWAIT */
#define	MCR_FAILOK	0x0100		/* private, for internal use only */
#define	MCR_TRYHARD	0x0200		/* private, for internal use only */
#define	MCR_USR1	0x1000		/* private, for internal use only */

#define	MCR_NONBLOCKING	(MCR_NOSLEEP | MCR_FAILOK | MCR_TRYHARD)

/*
 * Generic one-way linked list element structure.  This is used to handle
 * mcache_alloc_ext() requests in order to chain the allocated objects
 * together before returning them to the caller.
 */
typedef struct mcache_obj {
	struct mcache_obj	*obj_next;
} mcache_obj_t;

typedef struct mcache_bkt {
	void		*bkt_next;	/* next bucket in list */
	void		*bkt_obj[1];	/* one or more objects */
} mcache_bkt_t;

typedef struct mcache_bktlist {
	mcache_bkt_t	*bl_list;	/* bucket list */
	u_int32_t	bl_total;	/* number of buckets */
	u_int32_t	bl_min;		/* min since last update */
	u_int32_t	bl_reaplimit;	/* max reapable buckets */
	u_int64_t	bl_alloc;	/* allocations from this list */
} mcache_bktlist_t;

typedef struct mcache_bkttype {
	int		bt_bktsize;	/* bucket size (number of elements) */
	size_t		bt_minbuf;	/* all smaller buffers qualify */
	size_t		bt_maxbuf;	/* no larger bfufers qualify */
	struct mcache	*bt_cache;	/* bucket cache */
} mcache_bkttype_t;

typedef struct mcache_cpu {
	decl_lck_mtx_data(, cc_lock);
	mcache_bkt_t	*cc_filled;	/* the currently filled bucket */
	mcache_bkt_t	*cc_pfilled;	/* the previously filled bucket */
	u_int64_t	cc_alloc;	/* allocations from this cpu */
	u_int64_t	cc_free;	/* frees to this cpu */
	int		cc_objs;	/* number of objects in filled bkt */
	int		cc_pobjs;	/* number of objects in previous bkt */
	int		cc_bktsize;	/* number of elements in a full bkt */
} __attribute__((aligned(CPU_CACHE_SIZE), packed)) mcache_cpu_t;

typedef unsigned int (*mcache_allocfn_t)(void *, mcache_obj_t ***,
    unsigned int, int);
typedef void (*mcache_freefn_t)(void *, mcache_obj_t *, boolean_t);
typedef void (*mcache_auditfn_t)(void *, mcache_obj_t *, boolean_t);
typedef void (*mcache_logfn_t)(u_int32_t, mcache_obj_t *, boolean_t);
typedef void (*mcache_notifyfn_t)(void *, u_int32_t);

typedef struct mcache {
	/*
	 * Cache properties
	 */
	LIST_ENTRY(mcache) mc_list;	/* cache linkage */
	char		mc_name[32];	/* cache name */
	struct zone	*mc_slab_zone;	/* backend zone allocator */
	mcache_allocfn_t mc_slab_alloc;	/* slab layer allocate callback */
	mcache_freefn_t	mc_slab_free;	/* slab layer free callback */
	mcache_auditfn_t mc_slab_audit;	/* slab layer audit callback */
	mcache_logfn_t mc_slab_log;	/* slab layer log callback */
	mcache_notifyfn_t mc_slab_notify; /* slab layer notify callback */
	void		*mc_private;	/* opaque arg to callbacks */
	size_t		mc_bufsize;	/* object size */
	size_t		mc_align;	/* object alignment */
	u_int32_t	mc_flags;	/* cache creation flags */
	u_int32_t	mc_purge_cnt;	/* # of purges requested by slab */
	u_int32_t	mc_enable_cnt;	/* # of reenables due to purges */
	u_int32_t	mc_waiter_cnt;	/* # of slab layer waiters */
	u_int32_t	mc_wretry_cnt;	/* # of wait retries */
	u_int32_t	mc_nwretry_cnt;	/* # of no-wait retry attempts */
	u_int32_t	mc_nwfail_cnt;	/* # of no-wait retries that failed */
	decl_lck_mtx_data(, mc_sync_lock); /* protects purges and reenables */
	lck_attr_t	*mc_sync_lock_attr;
	lck_grp_t	*mc_sync_lock_grp;
	lck_grp_attr_t	*mc_sync_lock_grp_attr;
	/*
	 * Keep CPU and buckets layers lock statistics separate.
	 */
	lck_attr_t	*mc_cpu_lock_attr;
	lck_grp_t	*mc_cpu_lock_grp;
	lck_grp_attr_t	*mc_cpu_lock_grp_attr;

	/*
	 * Bucket layer common to all CPUs
	 */
	decl_lck_mtx_data(, mc_bkt_lock);
	lck_attr_t	*mc_bkt_lock_attr;
	lck_grp_t	*mc_bkt_lock_grp;
	lck_grp_attr_t  *mc_bkt_lock_grp_attr;
	mcache_bkttype_t *cache_bkttype;	/* bucket type */
	mcache_bktlist_t mc_full;		/* full buckets */
	mcache_bktlist_t mc_empty;		/* empty buckets */
	size_t		mc_chunksize;		/* bufsize + alignment */
	u_int32_t	mc_bkt_contention;	/* lock contention count */
	u_int32_t	mc_bkt_contention_prev;	/* previous snapshot */

	/*
	 * Per-CPU layer, aligned at cache line boundary
	 */
	mcache_cpu_t	mc_cpu[1];
} mcache_t;

#define	MCACHE_ALIGN	8	/* default guaranteed alignment */

/* Valid values for mc_flags */
#define	MCF_VERIFY	0x00000001	/* enable verification */
#define	MCF_TRACE	0x00000002	/* enable transaction auditing */
#define	MCF_NOCPUCACHE	0x00000010	/* disable CPU layer caching */
#define	MCF_NOLEAKLOG	0x00000100	/* disable leak logging */
#define	MCF_EXPLEAKLOG	0x00000200	/* expose leak info to user space */

#define	MCF_DEBUG	(MCF_VERIFY | MCF_TRACE)
#define	MCF_FLAGS_MASK	\
	(MCF_DEBUG | MCF_NOCPUCACHE | MCF_NOLEAKLOG | MCF_EXPLEAKLOG)

/* Valid values for notify callback */
#define	MCN_RETRYALLOC	0x00000001	/* Allocation should be retried */

#define	MCACHE_STACK_DEPTH 16

typedef struct mcache_audit {
	struct mcache_audit *mca_next;	/* next audit struct */
	void		*mca_addr;	/* address of buffer */
	mcache_t	*mca_cache;	/* parent cache of the buffer */
	struct thread	*mca_thread;	/* thread doing transaction */
	struct thread	*mca_pthread;	/* previous transaction thread */
	size_t		mca_contents_size; /* size of contents */
	void		*mca_contents;	/* contents at last free */
	uint16_t	mca_depth;	/* pc stack depth */
	uint16_t	mca_pdepth;	/* previous transaction pc stack */
	void		*mca_stack[MCACHE_STACK_DEPTH];
	void		*mca_pstack[MCACHE_STACK_DEPTH];
	void		*mca_uptr;	/* user-specific pointer */
	uint32_t	mca_uflags;	/* user-specific flags */
} mcache_audit_t;

__private_extern__ int assfail(const char *, const char *, int);
__private_extern__ void mcache_init(void);
__private_extern__ unsigned int mcache_getflags(void);
__private_extern__ mcache_t *mcache_create(const char *, size_t,
    size_t, u_int32_t, int);
__private_extern__ void *mcache_alloc(mcache_t *, int);
__private_extern__ void mcache_free(mcache_t *, void *);
__private_extern__ mcache_t *mcache_create_ext(const char *, size_t,
    mcache_allocfn_t, mcache_freefn_t, mcache_auditfn_t, mcache_logfn_t,
    mcache_notifyfn_t, void *, u_int32_t, int);
__private_extern__ void mcache_destroy(mcache_t *);
__private_extern__ unsigned int mcache_alloc_ext(mcache_t *, mcache_obj_t **,
    unsigned int, int);
__private_extern__ void mcache_free_ext(mcache_t *, mcache_obj_t *);
__private_extern__ void mcache_reap(void);
__private_extern__ boolean_t mcache_purge_cache(mcache_t *);
__private_extern__ void mcache_waiter_inc(mcache_t *);
__private_extern__ void mcache_waiter_dec(mcache_t *);
__private_extern__ boolean_t mcache_bkt_isempty(mcache_t *);

__private_extern__ void mcache_buffer_log(mcache_audit_t *, void *, mcache_t *);
__private_extern__ void mcache_set_pattern(u_int64_t, void *, size_t);
__private_extern__ void *mcache_verify_pattern(u_int64_t, void *, size_t);
__private_extern__ void *mcache_verify_set_pattern(u_int64_t, u_int64_t,
    void *, size_t);
__private_extern__ void mcache_audit_free_verify(mcache_audit_t *,
    void *, size_t, size_t);
__private_extern__ void mcache_audit_free_verify_set(mcache_audit_t *,
    void *, size_t, size_t);
__private_extern__ char *mcache_dump_mca(mcache_audit_t *);
__private_extern__ void mcache_audit_panic(mcache_audit_t *, void *, size_t,
    int64_t, int64_t);

__private_extern__ mcache_t *mcache_audit_cache;

#ifdef  __cplusplus
}
#endif

#endif /* KERNEL_PRIVATE */

#endif /* _SYS_MCACHE_H */
