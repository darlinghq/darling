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


#ifndef _I386_PMAP_INTERNAL_
#define _I386_PMAP_INTERNAL_
#ifdef MACH_KERNEL_PRIVATE

#include <vm/pmap.h>
#include <sys/kdebug.h>
#include <kern/ledger.h>
#include <kern/simple_lock.h>
#include <i386/bit_routines.h>

/*
 * pmap locking
 */

static inline void
PMAP_LOCK_EXCLUSIVE(pmap_t p)
{
	mp_disable_preemption();
	lck_rw_lock_exclusive(&p->pmap_rwl);
}

static inline void
PMAP_LOCK_SHARED(pmap_t p)
{
	mp_disable_preemption();
	lck_rw_lock_shared(&p->pmap_rwl);
}

static inline void
PMAP_LOCK_SHARED_TO_EXCLUSIVE(pmap_t p)
{
	lck_rw_lock_shared_to_exclusive(&p->pmap_rwl);
}

static inline void
PMAP_LOCK_EXCLUSIVE_TO_SHARED(pmap_t p)
{
	lck_rw_lock_exclusive_to_shared(&p->pmap_rwl);
}

static inline void
PMAP_UNLOCK_EXCLUSIVE(pmap_t p)
{
	lck_rw_unlock_exclusive(&p->pmap_rwl);
	mp_enable_preemption();
}

static inline void
PMAP_UNLOCK_SHARED(pmap_t p)
{
	lck_rw_unlock_shared(&p->pmap_rwl);
	mp_enable_preemption();
}

#define iswired(pte)    ((pte) & INTEL_PTE_WIRED)

#ifdef  PMAP_TRACES
extern  boolean_t       pmap_trace;
#define PMAP_TRACE(...) \
	if (pmap_trace) { \
	        KDBG_RELEASE(__VA_ARGS__); \
	}
#else
#define PMAP_TRACE(...) KDBG_DEBUG(__VA_ARGS__)
#endif /* PMAP_TRACES */

#define PMAP_TRACE_CONSTANT(...) KDBG_RELEASE(__VA_ARGS__)

kern_return_t   pmap_expand_pml4(
	pmap_t          map,
	vm_map_offset_t v,
	unsigned int options);

kern_return_t   pmap_expand_pdpt(
	pmap_t          map,
	vm_map_offset_t v,
	unsigned int options);

void            phys_attribute_set(
	ppnum_t         phys,
	int             bits);

void            pmap_set_reference(
	ppnum_t pn);

boolean_t       phys_page_exists(
	ppnum_t pn);

void
    pmap_flush_tlbs(pmap_t, vm_map_offset_t, vm_map_offset_t, int, pmap_flush_context *);

void
    pmap_update_cache_attributes_locked(ppnum_t, unsigned);


static inline void
PMAP_UPDATE_TLBS(pmap_t fp, addr64_t s, addr64_t e)
{
	pmap_flush_tlbs(fp, s, e, 0, NULL);
}

#define PMAP_DELAY_TLB_FLUSH            0x01

static inline void
PMAP_UPDATE_TLBS_DELAYED(pmap_t fp, addr64_t s, addr64_t e, pmap_flush_context *pfc)
{
	pmap_flush_tlbs(fp, s, e, PMAP_DELAY_TLB_FLUSH, pfc);
}

/*
 *	Private data structures.
 */

/*
 *	For each vm_page_t, there is a list of all currently
 *	valid virtual mappings of that page.  An entry is
 *	a pv_rooted_entry_t; the list is the pv_table.
 *
 *      N.B.  with the new combo rooted/hashed scheme it is
 *      only possibly to remove individual non-rooted entries
 *      if they are found via the hashed chains as there is no
 *      way to unlink the singly linked hashed entries if navigated to
 *      via the queue list off the rooted entries.  Think of it as
 *      hash/walk/pull, keeping track of the prev pointer while walking
 *      the singly linked hash list.  All of this is to save memory and
 *      keep both types of pv_entries as small as possible.
 */

/*
 *
 *  PV HASHING Changes - JK 1/2007
 *
 *  Pve's establish physical to virtual mappings.  These are used for aliasing of a
 *  physical page to (potentially many) virtual addresses within pmaps. In the
 *  previous implementation the structure of the pv_entries (each 16 bytes in size) was
 *
 *  typedef struct pv_entry {
 *   struct pv_entry_t    next;
 *   pmap_t                    pmap;
 *   vm_map_offset_t   va;
 *  } *pv_entry_t;
 *
 *  An initial array of these is created at boot time, one per physical page of
 *  memory, indexed by the physical page number. Additionally, a pool of entries
 *  is created from a pv_zone to be used as needed by pmap_enter() when it is
 *  creating new mappings.  Originally, we kept this pool around because the code
 *  in pmap_enter() was unable to block if it needed an entry and none were
 *  available - we'd panic.  Some time ago I restructured the pmap_enter() code
 *  so that for user pmaps it can block while zalloc'ing a pv structure and restart,
 *  removing a panic from the code (in the case of the kernel pmap we cannot block
 *  and still panic, so, we keep a separate hot pool for use only on kernel pmaps).
 *  The pool has not been removed since there is a large performance gain keeping
 *  freed pv's around for reuse and not suffering the overhead of zalloc for every
 *  new pv we need.
 *
 *  As pmap_enter() created new mappings it linked the new pve's for them off the
 *  fixed pv array for that ppn (off the next pointer).  These pve's are accessed
 *  for several operations, one of them being address space teardown. In that case,
 *  we basically do this
 *
 *       for (every page/pte in the space) {
 *               calc pve_ptr from the ppn in the pte
 *               for (every pv in the list for the ppn) {
 *                       if (this pv is for this pmap/vaddr) {
 *                               do housekeeping
 *                               unlink/free the pv
 *                       }
 *               }
 *       }
 *
 *  The problem arose when we were running, say 8000 (or even 2000) apache or
 *  other processes and one or all terminate. The list hanging off each pv array
 *  entry could have thousands of entries.  We were continuously linearly searching
 *  each of these lists as we stepped through the address space we were tearing
 *  down.  Because of the locks we hold, likely taking a cache miss for each node,
 *  and interrupt disabling for MP issues the system became completely unresponsive
 *  for many seconds while we did this.
 *
 *  Realizing that pve's are accessed in two distinct ways (linearly running the
 *  list by ppn for operations like pmap_page_protect and finding and
 *  modifying/removing a single pve as part of pmap_enter processing) has led to
 *  modifying the pve structures and databases.
 *
 *  There are now two types of pve structures.  A "rooted" structure which is
 *  basically the original structure accessed in an array by ppn, and a ''hashed''
 *  structure accessed on a hash list via a hash of [pmap, vaddr]. These have been
 *  designed with the two goals of minimizing wired memory and making the lookup of
 *  a ppn faster.  Since a vast majority of pages in the system are not aliased
 *  and hence represented by a single pv entry I've kept the rooted entry size as
 *  small as possible because there is one of these dedicated for every physical
 *  page of memory.  The hashed pve's are larger due to the addition of the hash
 *  link and the ppn entry needed for matching while running the hash list to find
 *  the entry we are looking for.  This way, only systems that have lots of
 *  aliasing (like 2000+ httpd procs) will pay the extra memory price. Both
 *  structures have the same first three fields allowing some simplification in
 *  the code.
 *
 *  They have these shapes
 *
 *  typedef struct pv_rooted_entry {
 *       queue_head_t		qlink;
 *       vm_map_offset_t		va;
 *       pmap_t			pmap;
 *  } *pv_rooted_entry_t;
 *
 *
 *  typedef struct pv_hashed_entry {
 *       queue_head_t		qlink;
 *       vm_map_offset_t		va;
 *       pmap_t			pmap;
 *       ppnum_t                ppn;
 *       struct pv_hashed_entry *nexth;
 *  } *pv_hashed_entry_t;
 *
 *  The main flow difference is that the code is now aware of the rooted entry and
 *  the hashed entries.  Code that runs the pv list still starts with the rooted
 *  entry and then continues down the qlink onto the hashed entries.  Code that is
 *  looking up a specific pv entry first checks the rooted entry and then hashes
 *  and runs the hash list for the match. The hash list lengths are much smaller
 *  than the original pv lists that contained all aliases for the specific ppn.
 *
 */

typedef struct pv_rooted_entry {
	/* first three entries must match pv_hashed_entry_t */
	queue_head_t            qlink;
	vm_map_offset_t         va_and_flags;   /* virtual address for mapping */
	pmap_t                  pmap;   /* pmap where mapping lies */
} *pv_rooted_entry_t;

#define PV_ROOTED_ENTRY_NULL    ((pv_rooted_entry_t) 0)

typedef struct pv_hashed_entry {
	/* first three entries must match pv_rooted_entry_t */
	queue_head_t            qlink;
	vm_map_offset_t         va_and_flags;
	pmap_t                  pmap;
	ppnum_t                 ppn;
	struct pv_hashed_entry  *nexth;
} *pv_hashed_entry_t;

#define PV_HASHED_ENTRY_NULL ((pv_hashed_entry_t)0)

#define PVE_VA(pve) ((pve)->va_and_flags & ~PAGE_MASK)
#define PVE_FLAGS(pve) ((pve)->va_and_flags & PAGE_MASK)
#define PVE_IS_ALTACCT 0x001
#define PVE_IS_ALTACCT_PAGE(pve) \
	(((pve)->va_and_flags & PVE_IS_ALTACCT) ? TRUE : FALSE)

//#define PV_DEBUG 1   /* uncomment to enable some PV debugging code */
#ifdef PV_DEBUG
#define CHK_NPVHASH() if(0 == npvhashmask) panic("npvhash uninitialized");
#else
#define CHK_NPVHASH(x)
#endif

#define NPVHASHBUCKETS (4096)
#define NPVHASHMASK ((NPVHASHBUCKETS) - 1) /* MUST BE 2^N - 1 */
#define PV_HASHED_LOW_WATER_MARK_DEFAULT 5000
#define PV_HASHED_KERN_LOW_WATER_MARK_DEFAULT 2000
#define PV_HASHED_ALLOC_CHUNK_INITIAL 2000
#define PV_HASHED_KERN_ALLOC_CHUNK_INITIAL 200

extern volatile uint32_t        mappingrecurse;
extern uint32_t  pv_hashed_low_water_mark, pv_hashed_kern_low_water_mark;

/*
 * PV hash locking
 */

#define LOCK_PV_HASH(hash)      lock_hash_hash(hash)
#define UNLOCK_PV_HASH(hash)    unlock_hash_hash(hash)
extern uint32_t npvhashmask;
extern pv_hashed_entry_t        *pv_hash_table;  /* hash lists */
extern pv_hashed_entry_t        pv_hashed_free_list;
extern pv_hashed_entry_t        pv_hashed_kern_free_list;
decl_simple_lock_data(extern, pv_hashed_free_list_lock);
decl_simple_lock_data(extern, pv_hashed_kern_free_list_lock);
decl_simple_lock_data(extern, pv_hash_table_lock);
decl_simple_lock_data(extern, phys_backup_lock);

extern zone_t           pv_hashed_list_zone;    /* zone of pv_hashed_entry
                                                 * structures */

extern uint32_t         pv_hashed_free_count;
extern uint32_t         pv_hashed_kern_free_count;
/*
 *	Each entry in the pv_head_table is locked by a bit in the
 *	pv_lock_table.  The lock bits are accessed by the address of
 *	the frame they lock.
 */
#define pv_lock_table_size(n)   (((n)+BYTE_SIZE-1)/BYTE_SIZE)
#define pv_hash_lock_table_size(n)  (((n)+BYTE_SIZE-1)/BYTE_SIZE)
extern char             *pv_lock_table;         /* pointer to array of bits */
extern char             *pv_hash_lock_table;
extern pv_rooted_entry_t pv_head_table; /* array of entries, one per page */

extern event_t mapping_replenish_event;

static inline void
PV_HASHED_ALLOC(pv_hashed_entry_t *pvh_ep)
{
	pmap_assert(*pvh_ep == PV_HASHED_ENTRY_NULL);
	simple_lock(&pv_hashed_free_list_lock, LCK_GRP_NULL);
	/* If the kernel reserved pool is low, let non-kernel mappings allocate
	 * synchronously, possibly subject to a throttle.
	 */
	if ((pv_hashed_kern_free_count > pv_hashed_kern_low_water_mark) && ((*pvh_ep = pv_hashed_free_list) != 0)) {
		pv_hashed_free_list = (pv_hashed_entry_t)(*pvh_ep)->qlink.next;
		pv_hashed_free_count--;
	}

	simple_unlock(&pv_hashed_free_list_lock);

	if (pv_hashed_free_count <= pv_hashed_low_water_mark) {
		if (!mappingrecurse && os_atomic_cmpxchg(&mappingrecurse, 0, 1, acq_rel)) {
			thread_wakeup(&mapping_replenish_event);
		}
	}
}

static inline void
PV_HASHED_FREE_LIST(pv_hashed_entry_t pvh_eh, pv_hashed_entry_t pvh_et, int pv_cnt)
{
	simple_lock(&pv_hashed_free_list_lock, LCK_GRP_NULL);
	pvh_et->qlink.next = (queue_entry_t)pv_hashed_free_list;
	pv_hashed_free_list = pvh_eh;
	pv_hashed_free_count += pv_cnt;
	simple_unlock(&pv_hashed_free_list_lock);
}

extern unsigned pmap_kern_reserve_alloc_stat;

static inline void
PV_HASHED_KERN_ALLOC(pv_hashed_entry_t *pvh_e)
{
	pmap_assert(*pvh_e == PV_HASHED_ENTRY_NULL);
	simple_lock(&pv_hashed_kern_free_list_lock, LCK_GRP_NULL);

	if ((*pvh_e = pv_hashed_kern_free_list) != 0) {
		pv_hashed_kern_free_list = (pv_hashed_entry_t)(*pvh_e)->qlink.next;
		pv_hashed_kern_free_count--;
		pmap_kern_reserve_alloc_stat++;
	}

	simple_unlock(&pv_hashed_kern_free_list_lock);

	if (pv_hashed_kern_free_count < pv_hashed_kern_low_water_mark) {
		if (!mappingrecurse && os_atomic_cmpxchg(&mappingrecurse, 0, 1, acq_rel)) {
			thread_wakeup(&mapping_replenish_event);
		}
	}
}

static inline void
PV_HASHED_KERN_FREE_LIST(pv_hashed_entry_t pvh_eh, pv_hashed_entry_t pvh_et, int pv_cnt)
{
	simple_lock(&pv_hashed_kern_free_list_lock, LCK_GRP_NULL);
	pvh_et->qlink.next = (queue_entry_t)pv_hashed_kern_free_list;
	pv_hashed_kern_free_list = pvh_eh;
	pv_hashed_kern_free_count += pv_cnt;
	simple_unlock(&pv_hashed_kern_free_list_lock);
}

extern uint64_t pmap_pv_throttle_stat, pmap_pv_throttled_waiters;
extern event_t pmap_user_pv_throttle_event;

static inline void
pmap_pv_throttle(__unused pmap_t p)
{
	pmap_assert(p != kernel_pmap);
	/* Apply throttle on non-kernel mappings */
	if (pv_hashed_kern_free_count < (pv_hashed_kern_low_water_mark / 2)) {
		pmap_pv_throttle_stat++;
		/* This doesn't need to be strictly accurate, merely a hint
		 * to eliminate the timeout when the reserve is replenished.
		 */
		pmap_pv_throttled_waiters++;
		assert_wait_timeout(&pmap_user_pv_throttle_event, THREAD_UNINT, 1, 1000 * NSEC_PER_USEC);
		thread_block(THREAD_CONTINUE_NULL);
	}
}

/*
 *	Index into pv_head table, its lock bits, and the modify/reference and managed bits
 */

#define pa_index(pa)            (i386_btop(pa))
#define ppn_to_pai(ppn)         ((int)ppn)

#define pai_to_pvh(pai)         (&pv_head_table[pai])
#define lock_pvh_pai(pai)       bit_lock(pai, (void *)pv_lock_table)
#define unlock_pvh_pai(pai)     bit_unlock(pai, (void *)pv_lock_table)
#define pvhash(idx)             (&pv_hash_table[idx])
#define lock_hash_hash(hash)    bit_lock(hash, (void *)pv_hash_lock_table)
#define unlock_hash_hash(hash)  bit_unlock(hash, (void *)pv_hash_lock_table)

#define IS_MANAGED_PAGE(x)                              \
	((unsigned int)(x) <= last_managed_page &&      \
	 (pmap_phys_attributes[x] & PHYS_MANAGED))
#define IS_INTERNAL_PAGE(x)                     \
	(IS_MANAGED_PAGE(x) && (pmap_phys_attributes[x] & PHYS_INTERNAL))
#define IS_REUSABLE_PAGE(x)                     \
	(IS_MANAGED_PAGE(x) && (pmap_phys_attributes[x] & PHYS_REUSABLE))
#define IS_ALTACCT_PAGE(x, pve)                          \
	(IS_MANAGED_PAGE((x)) &&                        \
	 (PVE_IS_ALTACCT_PAGE((pve))))

/*
 *	Physical page attributes.  Copy bits from PTE definition.
 */
#define PHYS_MODIFIED   INTEL_PTE_MOD   /* page modified */
#define PHYS_REFERENCED INTEL_PTE_REF   /* page referenced */
#define PHYS_MANAGED    INTEL_PTE_VALID /* page is managed */
#define PHYS_NOENCRYPT  INTEL_PTE_USER  /* no need to encrypt this page in the hibernation image */
#define PHYS_NCACHE     INTEL_PTE_NCACHE
#define PHYS_PAT        INTEL_PTE_PAT
#define PHYS_CACHEABILITY_MASK (INTEL_PTE_PAT | INTEL_PTE_NCACHE)
#define PHYS_INTERNAL   INTEL_PTE_WTHRU /* page from internal object */
#define PHYS_REUSABLE   INTEL_PTE_WRITE /* page is "reusable" */

#if DEVELOPMENT || DEBUG
extern boolean_t        pmap_disable_kheap_nx;
extern boolean_t        pmap_disable_kstack_nx;
#endif

#define PMAP_EXPAND_OPTIONS_NONE (0x0)
#define PMAP_EXPAND_OPTIONS_NOWAIT (PMAP_OPTIONS_NOWAIT)
#define PMAP_EXPAND_OPTIONS_NOENTER (PMAP_OPTIONS_NOENTER)
#define PMAP_EXPAND_OPTIONS_ALIASMAP (0x40000000U)
/*
 *	Amount of virtual memory mapped by one
 *	page-directory entry.
 */
#define PDE_MAPPED_SIZE         (pdetova(1))

/*
 *	Locking and TLB invalidation
 */

/*
 *	Locking Protocols: (changed 2/2007 JK)
 *
 *	There are two structures in the pmap module that need locking:
 *	the pmaps themselves, and the per-page pv_lists (which are locked
 *	by locking the pv_lock_table entry that corresponds to the pv_head
 *	for the list in question.)  Most routines want to lock a pmap and
 *	then do operations in it that require pv_list locking -- however
 *	pmap_remove_all and pmap_copy_on_write operate on a physical page
 *	basis and want to do the locking in the reverse order, i.e. lock
 *	a pv_list and then go through all the pmaps referenced by that list.
 *
 *      The system wide pmap lock has been removed. Now, paths take a lock
 *      on the pmap before changing its 'shape' and the reverse order lockers
 *      (coming in by phys ppn) take a lock on the corresponding pv and then
 *      retest to be sure nothing changed during the window before they locked
 *      and can then run up/down the pv lists holding the list lock. This also
 *      lets the pmap layer run (nearly completely) interrupt enabled, unlike
 *      previously.
 */

/*
 * PV locking
 */

#define LOCK_PVH(index) {               \
	mp_disable_preemption();        \
	lock_pvh_pai(index);            \
}

#define UNLOCK_PVH(index) {             \
	unlock_pvh_pai(index);          \
	mp_enable_preemption();         \
}

extern uint64_t pde_mapped_size;

extern char             *pmap_phys_attributes;
extern ppnum_t          last_managed_page;

/*
 * Used to record high memory allocated to kernel before
 * pmap_init() gets called.
 */
extern ppnum_t pmap_high_used_top;
extern ppnum_t pmap_high_used_bottom;
extern ppnum_t pmap_middle_used_top;
extern ppnum_t pmap_middle_used_bottom;

/*
 * when spinning through pmap_remove
 * ensure that we don't spend too much
 * time with preemption disabled.
 * I'm setting the current threshold
 * to 20us
 */
#define MAX_PREEMPTION_LATENCY_NS 20000
extern uint64_t max_preemption_latency_tsc;

#if DEBUG
#define PMAP_INTR_DEBUG (1)
#endif

#if PMAP_INTR_DEBUG
#define pmap_intr_assert() {                                                    \
	if (processor_avail_count > 1 && !ml_get_interrupts_enabled())          \
	        panic("pmap interrupt assert %d %s, %d", processor_avail_count, __FILE__, __LINE__); \
}
#else
#define pmap_intr_assert()
#endif
#if DEVELOPMENT || DEBUG
extern int              nx_enabled;
#endif
extern unsigned int    inuse_ptepages_count;

static inline uint32_t
pvhashidx(pmap_t pmap, vm_map_offset_t va)
{
	uint32_t hashidx = ((uint32_t)(uintptr_t)pmap ^
	    ((uint32_t)(va >> PAGE_SHIFT) & 0xFFFFFFFF)) &
	    npvhashmask;
	return hashidx;
}

/*
 * unlinks the pv_hashed_entry_t pvh from the singly linked hash chain.
 * properly deals with the anchor.
 * must be called with the hash locked, does not unlock it
 */
static inline void
pmap_pvh_unlink(pv_hashed_entry_t pvh)
{
	pv_hashed_entry_t       curh;
	pv_hashed_entry_t       *pprevh;
	int                     pvhash_idx;

	CHK_NPVHASH();
	pvhash_idx = pvhashidx(pvh->pmap, PVE_VA(pvh));

	pprevh = pvhash(pvhash_idx);

#if PV_DEBUG
	if (NULL == *pprevh) {
		panic("pvh_unlink null anchor"); /* JK DEBUG */
	}
#endif
	curh = *pprevh;

	while (PV_HASHED_ENTRY_NULL != curh) {
		if (pvh == curh) {
			break;
		}
		pprevh = &curh->nexth;
		curh = curh->nexth;
	}
	if (PV_HASHED_ENTRY_NULL == curh) {
		panic("pmap_pvh_unlink no pvh");
	}
	*pprevh = pvh->nexth;
	return;
}

static inline void
pv_hash_add(pv_hashed_entry_t   pvh_e,
    pv_rooted_entry_t   pv_h)
{
	pv_hashed_entry_t       *hashp;
	int                     pvhash_idx;

	CHK_NPVHASH();
	pvhash_idx = pvhashidx(pvh_e->pmap, PVE_VA(pvh_e));
	LOCK_PV_HASH(pvhash_idx);
	insque(&pvh_e->qlink, &pv_h->qlink);
	hashp = pvhash(pvhash_idx);
#if PV_DEBUG
	if (NULL == hashp) {
		panic("pv_hash_add(%p) null hash bucket", pvh_e);
	}
#endif
	pvh_e->nexth = *hashp;
	*hashp = pvh_e;
	UNLOCK_PV_HASH(pvhash_idx);
}

static inline void
pv_hash_remove(pv_hashed_entry_t pvh_e)
{
	int                     pvhash_idx;

	CHK_NPVHASH();
	pvhash_idx = pvhashidx(pvh_e->pmap, PVE_VA(pvh_e));
	LOCK_PV_HASH(pvhash_idx);
	remque(&pvh_e->qlink);
	pmap_pvh_unlink(pvh_e);
	UNLOCK_PV_HASH(pvhash_idx);
}

static inline boolean_t
popcnt1(uint64_t distance)
{
	return (distance & (distance - 1)) == 0;
}

/*
 * Routines to handle suppression of/recovery from some forms of pagetable corruption
 * incidents observed in the field. These can be either software induced (wild
 * stores to the mapwindows where applicable, use after free errors
 * (typically of pages addressed physically), mis-directed DMAs etc., or due
 * to DRAM/memory hierarchy/interconnect errors. Given the theoretical rarity of these errors,
 * the recording mechanism is deliberately not MP-safe. The overarching goal is to
 * still assert on potential software races, but attempt recovery from incidents
 * identifiable as occurring due to issues beyond the control of the pmap module.
 * The latter includes single-bit errors and malformed pagetable entries.
 * We currently limit ourselves to recovery/suppression of one incident per
 * PMAP_PAGETABLE_CORRUPTION_INTERVAL seconds, and details of the incident
 * are logged.
 * Assertions are not suppressed if kernel debugging is enabled. (DRK 09)
 */

typedef enum {
	PTE_VALID                = 0x0,
	PTE_INVALID              = 0x1,
	PTE_RSVD                 = 0x2,
	PTE_SUPERVISOR           = 0x4,
	PTE_BITFLIP              = 0x8,
	PV_BITFLIP               = 0x10,
	PTE_INVALID_CACHEABILITY = 0x20,
	PTE_NXBITFLIP            = 0x40
} pmap_pagetable_corruption_t;

typedef enum {
	ROOT_PRESENT = 0,
	ROOT_ABSENT = 1
} pmap_pv_assertion_t;

typedef enum {
	PMAP_ACTION_IGNORE      = 0x0,
	PMAP_ACTION_ASSERT      = 0x1,
	PMAP_ACTION_RETRY       = 0x2,
	PMAP_ACTION_RETRY_RELOCK = 0x4
} pmap_pagetable_corruption_action_t;

#define PMAP_PAGETABLE_CORRUPTION_INTERVAL (6ULL * 3600ULL)
extern uint64_t pmap_pagetable_corruption_interval_abstime;

extern uint32_t pmap_pagetable_corruption_incidents;
#define PMAP_PAGETABLE_CORRUPTION_MAX_LOG (8)
typedef struct {
	pmap_pv_assertion_t incident;
	pmap_pagetable_corruption_t reason;
	pmap_pagetable_corruption_action_t action;
	pmap_t  pmap;
	vm_map_offset_t vaddr;
	pt_entry_t pte;
	ppnum_t ppn;
	pmap_t pvpmap;
	vm_map_offset_t pvva;
	uint64_t abstime;
	int adj_ptes_count;
#define PMPTCR_MAX_ADJ_PTES (2)
	uint64_t adj_ptes[PMPTCR_MAX_ADJ_PTES];
} pmap_pagetable_corruption_record_t;

extern pmap_pagetable_corruption_record_t pmap_pagetable_corruption_records[];
extern uint64_t pmap_pagetable_corruption_last_abstime;
extern thread_call_t    pmap_pagetable_corruption_log_call;
extern boolean_t pmap_pagetable_corruption_timeout;

static inline pmap_pagetable_corruption_action_t
pmap_pagetable_corruption_log(pmap_pv_assertion_t incident, pmap_pagetable_corruption_t suppress_reason,
    pmap_pagetable_corruption_action_t action, pmap_t pmap, vm_map_offset_t vaddr, pt_entry_t *ptep,
    ppnum_t ppn, pmap_t pvpmap, vm_map_offset_t pvva, int adj_pteps_cnt, uint64_t **adj_pteps)
{
	uint32_t pmap_pagetable_corruption_log_index;
	uint64_t curtime = mach_absolute_time();

	if ((curtime - pmap_pagetable_corruption_last_abstime) < pmap_pagetable_corruption_interval_abstime) {
		pmap_pagetable_corruption_timeout = TRUE;
		action = PMAP_ACTION_ASSERT;
	} else {
		pmap_pagetable_corruption_last_abstime = curtime;
	}

	pmap_pagetable_corruption_log_index = pmap_pagetable_corruption_incidents++ % PMAP_PAGETABLE_CORRUPTION_MAX_LOG;
	pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].incident = incident;
	pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].reason = suppress_reason;
	pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].action = action;
	pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].pmap = pmap;
	pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].vaddr = vaddr;
	pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].pte = *ptep;
	pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].ppn = ppn;
	pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].pvpmap = pvpmap;
	pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].pvva = pvva;
	pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].abstime = curtime;
	if (adj_pteps_cnt > 0 && adj_pteps != NULL) {
		pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].adj_ptes_count = MIN(adj_pteps_cnt, PMPTCR_MAX_ADJ_PTES);
		for (int i = 0; i < pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].adj_ptes_count; i++) {
			pmap_pagetable_corruption_records[pmap_pagetable_corruption_log_index].adj_ptes[i] = *adj_pteps[i];
		}
	}
	/* Asynchronously log */
	thread_call_enter(pmap_pagetable_corruption_log_call);

	return action;
}

static inline pmap_pagetable_corruption_action_t
pmap_classify_pagetable_corruption(pmap_t pmap, vm_map_offset_t vaddr, ppnum_t *ppnp, pt_entry_t *ptep, pmap_pv_assertion_t incident)
{
	pmap_pagetable_corruption_action_t      action = PMAP_ACTION_ASSERT;
	pmap_pagetable_corruption_t     suppress_reason = PTE_VALID;
	ppnum_t                 suppress_ppn = 0;
	pt_entry_t cpte = *ptep;
	ppnum_t cpn = pa_index(pte_to_pa(cpte));
	ppnum_t ppn = *ppnp;
	pv_rooted_entry_t       pv_h = pai_to_pvh(ppn_to_pai(ppn));
	pv_rooted_entry_t       pv_e = pv_h;
	uint32_t        bitdex;
	pmap_t pvpmap = pv_h->pmap;
	vm_map_offset_t pvva = PVE_VA(pv_h);
	vm_map_offset_t pve_flags;
	boolean_t ppcd = FALSE;
	boolean_t is_ept;

	/* Ideally, we'd consult the Mach VM here to definitively determine
	 * the nature of the mapping for this address space and address.
	 * As that would be a layering violation in this context, we
	 * use various heuristics to recover from single bit errors,
	 * malformed pagetable entries etc. These are not intended
	 * to be comprehensive.
	 */

	/* As a precautionary measure, mark A+D */
	pmap_phys_attributes[ppn_to_pai(ppn)] |= (PHYS_MODIFIED | PHYS_REFERENCED);
	is_ept = is_ept_pmap(pmap);

	/*
	 * Correct potential single bit errors in either (but not both) element
	 * of the PV
	 */
	do {
		if ((popcnt1((uintptr_t)pv_e->pmap ^ (uintptr_t)pmap) && PVE_VA(pv_e) == vaddr) ||
		    (pv_e->pmap == pmap && popcnt1(PVE_VA(pv_e) ^ vaddr))) {
			pve_flags = PVE_FLAGS(pv_e);
			pv_e->pmap = pmap;
			pv_h->va_and_flags = vaddr | pve_flags;
			suppress_reason = PV_BITFLIP;
			action = PMAP_ACTION_RETRY;
			goto pmap_cpc_exit;
		}
	} while (((pv_e = (pv_rooted_entry_t) queue_next(&pv_e->qlink))) && (pv_e != pv_h));

	/* Discover root entries with a Hamming
	 * distance of 1 from the supplied
	 * physical page frame.
	 */
	for (bitdex = 0; bitdex < (sizeof(ppnum_t) << 3); bitdex++) {
		ppnum_t npn = cpn ^ (ppnum_t) (1ULL << bitdex);
		if (IS_MANAGED_PAGE(npn)) {
			pv_rooted_entry_t npv_h = pai_to_pvh(ppn_to_pai(npn));
			if (PVE_VA(npv_h) == vaddr && npv_h->pmap == pmap) {
				suppress_reason = PTE_BITFLIP;
				suppress_ppn = npn;
				action = PMAP_ACTION_RETRY_RELOCK;
				UNLOCK_PVH(ppn_to_pai(ppn));
				*ppnp = npn;
				goto pmap_cpc_exit;
			}
		}
	}

	if (pmap == kernel_pmap) {
		action = PMAP_ACTION_ASSERT;
		goto pmap_cpc_exit;
	}

	/*
	 * Check for malformed/inconsistent entries.
	 * The first check here isn't useful for EPT PTEs because INTEL_EPT_NCACHE == 0
	 */
	if (!is_ept && ((cpte & (INTEL_PTE_NCACHE | INTEL_PTE_WTHRU | INTEL_PTE_PAT)) == (INTEL_PTE_NCACHE | INTEL_PTE_WTHRU))) {
		action = PMAP_ACTION_IGNORE;
		suppress_reason = PTE_INVALID_CACHEABILITY;
	} else if (cpte & INTEL_PTE_RSVD) {
		action = PMAP_ACTION_IGNORE;
		suppress_reason = PTE_RSVD;
	} else if ((pmap != kernel_pmap) && (!is_ept) && ((cpte & INTEL_PTE_USER) == 0)) {
		action = PMAP_ACTION_IGNORE;
		suppress_reason = PTE_SUPERVISOR;
	}
pmap_cpc_exit:
	PE_parse_boot_argn("-pmap_pagetable_corruption_deassert", &ppcd, sizeof(ppcd));

	if (debug_boot_arg && !ppcd) {
		action = PMAP_ACTION_ASSERT;
	}

	return pmap_pagetable_corruption_log(incident, suppress_reason, action, pmap, vaddr, &cpte, *ppnp, pvpmap, pvva, 0, 0);
}

static inline boolean_t
pmap_compressed_pte_corruption_repair(uint64_t pte, uint64_t *pte_addr, uint64_t *ptep, pmap_t pmap,
    vm_map_offset_t vaddr)
{
	uint64_t *adj_pteps[2];
	int pteidx = ((uintptr_t)ptep & INTEL_OFFMASK) / sizeof(pt_entry_t);
	pmap_pagetable_corruption_action_t action = PMAP_ACTION_IGNORE;

	/*
	 * Grab pointers to PTEs on either side of the PTE in question, unless we're at the start of
	 * a PT (grab pointers to the next and next-next PTEs) or the end of a PT (grab the previous
	 * 2 PTEs).
	 */
	if (pteidx == 0) {
		adj_pteps[0] = ptep + 1;
		adj_pteps[1] = ptep + 2;
	} else if (pteidx == (NPTPG - 1)) {
		adj_pteps[0] = ptep - 2;
		adj_pteps[1] = ptep - 1;
	} else {
		adj_pteps[0] = ptep - 1;
		adj_pteps[1] = ptep + 1;
	}

	/*
	 * Since the compressed PTE no longer has a PTE associated, we cannot pass in the pv data to
	 * pmap_pagetable_corruption_log, so instead supply adjacent PTEs for logging.
	 */
	if (pmap_pagetable_corruption_log(ROOT_ABSENT, (pte & INTEL_PTE_NX) ? PTE_NXBITFLIP : PTE_BITFLIP,
	    action, pmap, vaddr, ptep, (ppnum_t)~0UL, 0, 0, sizeof(adj_pteps) / sizeof(adj_pteps[0]),
	    adj_pteps) != PMAP_ACTION_ASSERT) {
		/* Correct the flipped bit(s) and continue */
		pmap_store_pte(ptep, pte & INTEL_PTE_COMPRESSED_MASK);
		pmap->corrected_compressed_ptes_count++;
		return TRUE; /* Returning TRUE to indicate this is a now a valid compressed PTE (we hope) */
	}

	panic("compressed PTE %p 0x%llx has extra bits 0x%llx: corrupted? Adjacent PTEs: 0x%llx@%p, 0x%llx@%p",
	    pte_addr, pte, pte & ~INTEL_PTE_COMPRESSED_MASK, *adj_pteps[0], adj_pteps[0], *adj_pteps[1], adj_pteps[1]);
	/*NOTREACHED*/
}

/*
 * Remove pv list entry.
 * Called with pv_head_table entry locked.
 * Returns pv entry to be freed (or NULL).
 */
static inline __attribute__((always_inline)) pv_hashed_entry_t
pmap_pv_remove(pmap_t           pmap,
    vm_map_offset_t  vaddr,
    ppnum_t          *ppnp,
    pt_entry_t       *pte,
    boolean_t        *was_altacct)
{
	pv_hashed_entry_t       pvh_e;
	pv_rooted_entry_t       pv_h;
	pv_hashed_entry_t       *pprevh;
	int                     pvhash_idx;
	uint32_t                pv_cnt;
	ppnum_t                 ppn;

	*was_altacct = FALSE;
pmap_pv_remove_retry:
	ppn = *ppnp;
	pvh_e = PV_HASHED_ENTRY_NULL;
	pv_h = pai_to_pvh(ppn_to_pai(ppn));

	if (__improbable(pv_h->pmap == PMAP_NULL)) {
		pmap_pagetable_corruption_action_t pac = pmap_classify_pagetable_corruption(pmap, vaddr, ppnp, pte, ROOT_ABSENT);
		if (pac == PMAP_ACTION_IGNORE) {
			goto pmap_pv_remove_exit;
		} else if (pac == PMAP_ACTION_ASSERT) {
			panic("Possible memory corruption: pmap_pv_remove(%p,0x%llx,0x%x, 0x%llx, %p, %p): null pv_list, priors: %d", pmap, vaddr, ppn, *pte, ppnp, pte, pmap_pagetable_corruption_incidents);
		} else if (pac == PMAP_ACTION_RETRY_RELOCK) {
			LOCK_PVH(ppn_to_pai(*ppnp));
			pmap_phys_attributes[ppn_to_pai(*ppnp)] |= (PHYS_MODIFIED | PHYS_REFERENCED);
			goto pmap_pv_remove_retry;
		} else if (pac == PMAP_ACTION_RETRY) {
			goto pmap_pv_remove_retry;
		}
	}

	if (PVE_VA(pv_h) == vaddr && pv_h->pmap == pmap) {
		*was_altacct = IS_ALTACCT_PAGE(ppn_to_pai(*ppnp), pv_h);
		/*
		 * Header is the pv_rooted_entry.
		 * We can't free that. If there is a queued
		 * entry after this one we remove that
		 * from the ppn queue, we remove it from the hash chain
		 * and copy it to the rooted entry. Then free it instead.
		 */
		pvh_e = (pv_hashed_entry_t) queue_next(&pv_h->qlink);
		if (pv_h != (pv_rooted_entry_t) pvh_e) {
			/*
			 * Entry queued to root, remove this from hash
			 * and install as new root.
			 */
			CHK_NPVHASH();
			pvhash_idx = pvhashidx(pvh_e->pmap, PVE_VA(pvh_e));
			LOCK_PV_HASH(pvhash_idx);
			remque(&pvh_e->qlink);
			pprevh = pvhash(pvhash_idx);
			if (PV_HASHED_ENTRY_NULL == *pprevh) {
				panic("Possible memory corruption: pmap_pv_remove(%p,0x%llx,0x%x): "
				    "empty hash, removing rooted, priors: %d",
				    pmap, vaddr, ppn, pmap_pagetable_corruption_incidents);
			}
			pmap_pvh_unlink(pvh_e);
			UNLOCK_PV_HASH(pvhash_idx);
			pv_h->pmap = pvh_e->pmap;
			pv_h->va_and_flags = pvh_e->va_and_flags;
			/* dispose of pvh_e */
		} else {
			/* none queued after rooted */
			pv_h->pmap = PMAP_NULL;
			pvh_e = PV_HASHED_ENTRY_NULL;
		}
	} else {
		/*
		 * not removing rooted pv. find it on hash chain, remove from
		 * ppn queue and hash chain and free it
		 */
		CHK_NPVHASH();
		pvhash_idx = pvhashidx(pmap, vaddr);
		LOCK_PV_HASH(pvhash_idx);
		pprevh = pvhash(pvhash_idx);
		if (PV_HASHED_ENTRY_NULL == *pprevh) {
			panic("Possible memory corruption: pmap_pv_remove(%p,0x%llx,0x%x, 0x%llx, %p): empty hash, priors: %d",
			    pmap, vaddr, ppn, *pte, pte, pmap_pagetable_corruption_incidents);
		}
		pvh_e = *pprevh;
		pmap_pv_hashlist_walks++;
		pv_cnt = 0;
		while (PV_HASHED_ENTRY_NULL != pvh_e) {
			pv_cnt++;
			if (pvh_e->pmap == pmap &&
			    PVE_VA(pvh_e) == vaddr &&
			    pvh_e->ppn == ppn) {
				break;
			}
			pprevh = &pvh_e->nexth;
			pvh_e = pvh_e->nexth;
		}

		if (PV_HASHED_ENTRY_NULL == pvh_e) {
			pmap_pagetable_corruption_action_t pac = pmap_classify_pagetable_corruption(pmap, vaddr, ppnp, pte, ROOT_PRESENT);

			if (pac == PMAP_ACTION_ASSERT) {
				panic("Possible memory corruption: pmap_pv_remove(%p, 0x%llx, 0x%x, 0x%llx, %p, %p): pv not on hash, head: %p, 0x%llx, priors: %d", pmap, vaddr, ppn, *pte, ppnp, pte, pv_h->pmap, PVE_VA(pv_h), pmap_pagetable_corruption_incidents);
			} else {
				UNLOCK_PV_HASH(pvhash_idx);
				if (pac == PMAP_ACTION_RETRY_RELOCK) {
					LOCK_PVH(ppn_to_pai(*ppnp));
					pmap_phys_attributes[ppn_to_pai(*ppnp)] |= (PHYS_MODIFIED | PHYS_REFERENCED);
					goto pmap_pv_remove_retry;
				} else if (pac == PMAP_ACTION_RETRY) {
					goto pmap_pv_remove_retry;
				} else if (pac == PMAP_ACTION_IGNORE) {
					goto pmap_pv_remove_exit;
				}
			}
		}

		*was_altacct = IS_ALTACCT_PAGE(ppn_to_pai(*ppnp), pvh_e);

		pmap_pv_hashlist_cnts += pv_cnt;
		if (pmap_pv_hashlist_max < pv_cnt) {
			pmap_pv_hashlist_max = pv_cnt;
		}
		*pprevh = pvh_e->nexth;
		remque(&pvh_e->qlink);
		UNLOCK_PV_HASH(pvhash_idx);
	}
pmap_pv_remove_exit:
	return pvh_e;
}

static inline __attribute__((always_inline)) boolean_t
pmap_pv_is_altacct(
	pmap_t          pmap,
	vm_map_offset_t vaddr,
	ppnum_t         ppn)
{
	pv_hashed_entry_t       pvh_e;
	pv_rooted_entry_t       pv_h;
	int                     pvhash_idx;
	boolean_t               is_altacct;

	pvh_e = PV_HASHED_ENTRY_NULL;
	pv_h = pai_to_pvh(ppn_to_pai(ppn));

	if (__improbable(pv_h->pmap == PMAP_NULL)) {
		return FALSE;
	}

	if (PVE_VA(pv_h) == vaddr && pv_h->pmap == pmap) {
		/*
		 * Header is the pv_rooted_entry.
		 */
		return IS_ALTACCT_PAGE(ppn, pv_h);
	}

	CHK_NPVHASH();
	pvhash_idx = pvhashidx(pmap, vaddr);
	LOCK_PV_HASH(pvhash_idx);
	pvh_e = *(pvhash(pvhash_idx));
	while (PV_HASHED_ENTRY_NULL != pvh_e) {
		if (pvh_e->pmap == pmap &&
		    PVE_VA(pvh_e) == vaddr &&
		    pvh_e->ppn == ppn) {
			break;
		}
		pvh_e = pvh_e->nexth;
	}
	if (PV_HASHED_ENTRY_NULL == pvh_e) {
		is_altacct = FALSE;
	} else {
		is_altacct = IS_ALTACCT_PAGE(ppn, pvh_e);
	}
	UNLOCK_PV_HASH(pvhash_idx);

	return is_altacct;
}

extern int      pt_fake_zone_index;
static inline void
PMAP_ZINFO_PALLOC(pmap_t pmap, vm_size_t bytes)
{
	pmap_ledger_credit(pmap, task_ledgers.tkm_private, bytes);
}

static inline void
PMAP_ZINFO_PFREE(pmap_t pmap, vm_size_t bytes)
{
	pmap_ledger_debit(pmap, task_ledgers.tkm_private, bytes);
}

static inline void
PMAP_ZINFO_SALLOC(pmap_t pmap, vm_size_t bytes)
{
	pmap_ledger_credit(pmap, task_ledgers.tkm_shared, bytes);
}

static inline void
PMAP_ZINFO_SFREE(pmap_t pmap, vm_size_t bytes)
{
	pmap_ledger_debit(pmap, task_ledgers.tkm_shared, bytes);
}

extern boolean_t        pmap_initialized;/* Has pmap_init completed? */
#define valid_page(x) (pmap_initialized && pmap_valid_page(x))

int             phys_attribute_test(
	ppnum_t         phys,
	int             bits);
void            phys_attribute_clear(
	ppnum_t         phys,
	int             bits,
	unsigned int    options,
	void            *arg);

//#define PCID_DEBUG 1
#if     PCID_DEBUG
#define pmap_pcid_log(fmt, args...)                                     \
	do {                                                            \
	        kprintf(fmt, ##args);                                   \
	        printf(fmt, ##args);                                    \
	} while(0)
#else
#define pmap_pcid_log(fmt, args...)
#endif
void    pmap_pcid_configure(void);


/*
 * Atomic 64-bit compare and exchange of a page table entry.
 */

#include <machine/atomic.h>
static inline boolean_t
pmap_cmpx_pte(pt_entry_t *entryp, pt_entry_t old, pt_entry_t new)
{
	return __c11_atomic_compare_exchange_strong((_Atomic pt_entry_t *)entryp, &old, new,
	           memory_order_acq_rel_smp, memory_order_relaxed);
}

extern uint32_t pmap_update_clear_pte_count;

static inline void
pmap_update_pte(pt_entry_t *mptep, uint64_t pclear_bits, uint64_t pset_bits)
{
	pt_entry_t npte, opte;
	do {
		opte = *mptep;
		if (__improbable(opte == 0)) {
#if DEVELOPMENT || DEBUG
			pmap_update_clear_pte_count++;
#endif
			break;
		}
		npte = opte & ~(pclear_bits);
		npte |= pset_bits;
	}       while (!pmap_cmpx_pte(mptep, opte, npte));
}

/*
 * The single pml4 page per pmap is allocated at pmap create time and exists
 * for the duration of the pmap. we allocate this page in kernel vm.
 * this returns the address of the requested pml4 entry in the top level page.
 */
static inline
pml4_entry_t *
pmap64_pml4(pmap_t pmap, vm_map_offset_t vaddr)
{
	if (__improbable((vaddr > 0x00007FFFFFFFFFFFULL) &&
	    (vaddr < 0xFFFF800000000000ULL))) {
		return NULL;
	}

#if     DEBUG
	return PHYSMAP_PTOV(&((pml4_entry_t *)pmap->pm_cr3)[(vaddr >> PML4SHIFT) & (NPML4PG - 1)]);
#else
	return &pmap->pm_pml4[(vaddr >> PML4SHIFT) & (NPML4PG - 1)];
#endif
}

static inline pml4_entry_t *
pmap64_user_pml4(pmap_t pmap, vm_map_offset_t vaddr)
{
	if (__improbable((vaddr > 0x00007FFFFFFFFFFFULL) &&
	    (vaddr < 0xFFFF800000000000ULL))) {
		return NULL;
	}

#if     DEBUG
	return PHYSMAP_PTOV(&((pml4_entry_t *)pmap->pm_ucr3)[(vaddr >> PML4SHIFT) & (NPML4PG - 1)]);
#else
	return &pmap->pm_upml4[(vaddr >> PML4SHIFT) & (NPML4PG - 1)];
#endif
}

/*
 * Returns address of requested PDPT entry in the physmap.
 */
static inline pdpt_entry_t *
pmap64_pdpt(pmap_t pmap, vm_map_offset_t vaddr)
{
	pml4_entry_t    newpf;
	pml4_entry_t    *pml4;
	boolean_t       is_ept;

	pml4 = pmap64_pml4(pmap, vaddr);
	is_ept = is_ept_pmap(pmap);

	if (pml4 && (*pml4 & PTE_VALID_MASK(is_ept))) {
		newpf = *pml4 & PG_FRAME;
		return &((pdpt_entry_t *) PHYSMAP_PTOV(newpf))
		       [(vaddr >> PDPTSHIFT) & (NPDPTPG - 1)];
	}
	return NULL;
}
/*
 * Returns the address of the requested PDE entry in the physmap.
 */
static inline pd_entry_t *
pmap_pde_internal1(vm_map_offset_t vaddr, boolean_t is_ept, pdpt_entry_t *pdpte)
{
	if (*pdpte & PTE_VALID_MASK(is_ept)) {
		pdpt_entry_t    newpf = *pdpte & PG_FRAME;
		return &((pd_entry_t *) PHYSMAP_PTOV(newpf))
		       [(vaddr >> PDSHIFT) & (NPDPG - 1)];
	} else {
		return NULL;
	}
}

static inline pd_entry_t *
pmap_pde_internal0(pmap_t pmap, vm_map_offset_t vaddr, boolean_t is_ept)
{
	pdpt_entry_t    *pdpt;

	pdpt = pmap64_pdpt(pmap, vaddr);
	if (pdpt) {
		return pmap_pde_internal1(vaddr, is_ept, pdpt);
	} else {
		return NULL;
	}
}


static inline pd_entry_t *
pmap_pde(pmap_t pmap, vm_map_offset_t vaddr)
{
	pdpt_entry_t    *pdpt;
	boolean_t       is_ept;

	pdpt = pmap64_pdpt(pmap, vaddr);
	is_ept = is_ept_pmap(pmap);

	if (pdpt) {
		return pmap_pde_internal1(vaddr, is_ept, pdpt);
	} else {
		return NULL;
	}
}


/*
 * return address of mapped pte for vaddr va in pmap pmap.
 *
 * In case the pde maps a superpage, return the pde, which, in this case
 * is the actual page table entry.
 */


static inline pt_entry_t *
pmap_pte_internal(vm_map_offset_t vaddr, boolean_t is_ept, pd_entry_t *pde)
{
	if (*pde & PTE_VALID_MASK(is_ept)) {
		if (__improbable(*pde & PTE_PS)) {
			return pde;
		}
		pd_entry_t      newpf = *pde & PG_FRAME;

		return &((pt_entry_t *)PHYSMAP_PTOV(newpf))
		       [i386_btop(vaddr) & (ppnum_t)(NPTEPG - 1)];
	} else {
		return NULL;
	}
}

static inline pt_entry_t *
pmap_pte(pmap_t pmap, vm_map_offset_t vaddr)
{
	pd_entry_t      *pde;

	boolean_t       is_ept;

	is_ept = is_ept_pmap(pmap);

	pde = pmap_pde_internal0(pmap, vaddr, is_ept);

	if (pde) {
		return pmap_pte_internal(vaddr, is_ept, pde);
	} else {
		return NULL;
	}
}

extern void     pmap_alias(
	vm_offset_t     ava,
	vm_map_offset_t start,
	vm_map_offset_t end,
	vm_prot_t       prot,
	unsigned int options);

#if     DEBUG
#define DPRINTF(x...)   kprintf(x)
#else
#define DPRINTF(x...)
#endif

#endif /* MACH_KERNEL_PRIVATE */
#endif /* _I386_PMAP_INTERNAL_ */
