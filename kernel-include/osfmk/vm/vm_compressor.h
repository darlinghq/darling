/*
 * Copyright (c) 2000-2016 Apple Inc. All rights reserved.
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

#include <kern/kalloc.h>
#include <vm/vm_compressor_pager.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <vm/vm_protos.h>
#include <vm/WKdm_new.h>
#include <vm/vm_object.h>
#include <vm/vm_map.h>
#include <machine/pmap.h>
#include <kern/locks.h>

#include <sys/kdebug.h>

#if defined(__arm64__)
#include <arm/proc_reg.h>
#endif

#define C_SEG_OFFSET_BITS       16
#define C_SEG_BUFSIZE           (1024 * 256)
#define C_SEG_MAX_PAGES         (C_SEG_BUFSIZE / PAGE_SIZE)

#if CONFIG_EMBEDDED
#define C_SEG_OFF_LIMIT         (C_SEG_BYTES_TO_OFFSET((C_SEG_BUFSIZE - 512)))
#define C_SEG_ALLOCSIZE         (C_SEG_BUFSIZE + PAGE_SIZE)
#else
#define C_SEG_OFF_LIMIT         (C_SEG_BYTES_TO_OFFSET((C_SEG_BUFSIZE - 128)))
#define C_SEG_ALLOCSIZE         (C_SEG_BUFSIZE)
#endif
#define C_SEG_MAX_POPULATE_SIZE (4 * PAGE_SIZE)

#if defined(__arm64__)

#if DEVELOPMENT || DEBUG

#if defined(PLATFORM_WatchOS)
#define VALIDATE_C_SEGMENTS (1)
#endif
#endif

#endif


#if DEBUG || COMPRESSOR_INTEGRITY_CHECKS
#define ENABLE_SWAP_CHECKS 1
#define ENABLE_COMPRESSOR_CHECKS 1
#define POPCOUNT_THE_COMPRESSED_DATA (1)
#else
#define ENABLE_SWAP_CHECKS 0
#define ENABLE_COMPRESSOR_CHECKS 0
#endif

#define CHECKSUM_THE_SWAP               ENABLE_SWAP_CHECKS      /* Debug swap data */
#define CHECKSUM_THE_DATA               ENABLE_COMPRESSOR_CHECKS        /* Debug compressor/decompressor data */
#define CHECKSUM_THE_COMPRESSED_DATA    ENABLE_COMPRESSOR_CHECKS        /* Debug compressor/decompressor compressed data */

#ifndef VALIDATE_C_SEGMENTS
#define VALIDATE_C_SEGMENTS             ENABLE_COMPRESSOR_CHECKS        /* Debug compaction */
#endif

#define RECORD_THE_COMPRESSED_DATA      0

struct c_slot {
	uint64_t        c_offset:C_SEG_OFFSET_BITS,
#if defined(__arm64__)
	c_size:14,
	    c_codec:1,
	    c_packed_ptr:33;
#elif defined(__arm__)
	c_size:12,
	c_codec:1,
	c_packed_ptr:35;
#else
	c_size:12,
	c_packed_ptr:36;
#endif
#if CHECKSUM_THE_DATA
	unsigned int    c_hash_data;
#endif
#if CHECKSUM_THE_COMPRESSED_DATA
	unsigned int    c_hash_compressed_data;
#endif
#if POPCOUNT_THE_COMPRESSED_DATA
	unsigned int    c_pop_cdata;
#endif
};

#define C_IS_EMPTY              0
#define C_IS_FREE               1
#define C_IS_FILLING            2
#define C_ON_AGE_Q              3
#define C_ON_SWAPOUT_Q          4
#define C_ON_SWAPPEDOUT_Q       5
#define C_ON_SWAPPEDOUTSPARSE_Q 6
#define C_ON_SWAPPEDIN_Q        7
#define C_ON_MAJORCOMPACT_Q     8
#define C_ON_BAD_Q              9
#define C_ON_SWAPIO_Q          10


struct c_segment {
	lck_mtx_t       c_lock;
	queue_chain_t   c_age_list;
	queue_chain_t   c_list;

#define C_SEG_MAX_LIMIT         (1 << 20)       /* this needs to track the size of c_mysegno */
	uint32_t        c_mysegno:20,
	    c_busy:1,
	    c_busy_swapping:1,
	    c_wanted:1,
	    c_on_minorcompact_q:1,              /* can also be on the age_q, the majorcompact_q or the swappedin_q */

	    c_state:4,                          /* what state is the segment in which dictates which q to find it on */
	    c_overage_swap:1,
	    c_reserved:3;

	uint32_t        c_creation_ts;
	uint64_t        c_generation_id;

	int32_t         c_bytes_used;
	int32_t         c_bytes_unused;
	uint32_t        c_slots_used;

	uint16_t        c_firstemptyslot;
	uint16_t        c_nextslot;
	uint32_t        c_nextoffset;
	uint32_t        c_populated_offset;

	uint32_t        c_swappedin_ts;

	union {
		int32_t *c_buffer;
		uint64_t c_swap_handle;
	} c_store;

#if     VALIDATE_C_SEGMENTS
	uint32_t        c_was_minor_compacted;
	uint32_t        c_was_major_compacted;
	uint32_t        c_was_major_donor;
#endif
#if CHECKSUM_THE_SWAP
	unsigned int    cseg_hash;
	unsigned int    cseg_swap_size;
#endif /* CHECKSUM_THE_SWAP */

#if MACH_ASSERT
	thread_t        c_busy_for_thread;
#endif /* MACH_ASSERT */

	int             c_slot_var_array_len;
	struct  c_slot  *c_slot_var_array;
	struct  c_slot  c_slot_fixed_array[0];
};


struct  c_slot_mapping {
	uint32_t        s_cseg:22,      /* segment number + 1 */
	    s_cindx:10;                 /* index in the segment */
};
#define C_SLOT_MAX_INDEX        (1 << 10)

typedef struct c_slot_mapping *c_slot_mapping_t;


#define C_SEG_SLOT_VAR_ARRAY_MIN_LEN    C_SEG_MAX_PAGES

extern  int             c_seg_fixed_array_len;
extern  vm_offset_t     c_buffers;
#define C_SEG_BUFFER_ADDRESS(c_segno)   ((c_buffers + ((uint64_t)c_segno * (uint64_t)C_SEG_ALLOCSIZE)))

#define C_SEG_SLOT_FROM_INDEX(cseg, index)      (index < c_seg_fixed_array_len ? &(cseg->c_slot_fixed_array[index]) : &(cseg->c_slot_var_array[index - c_seg_fixed_array_len]))

#define C_SEG_OFFSET_TO_BYTES(off)      ((off) * (int) sizeof(int32_t))
#define C_SEG_BYTES_TO_OFFSET(bytes)    ((bytes) / (int) sizeof(int32_t))

#define C_SEG_UNUSED_BYTES(cseg)        (cseg->c_bytes_unused + (C_SEG_OFFSET_TO_BYTES(cseg->c_populated_offset - cseg->c_nextoffset)))
//todo opensource

#ifndef __PLATFORM_WKDM_ALIGNMENT_MASK__
#define C_SEG_OFFSET_ALIGNMENT_MASK     0x3ULL
#define C_SEG_OFFSET_ALIGNMENT_BOUNDARY 0x4
#else
#define C_SEG_OFFSET_ALIGNMENT_MASK     __PLATFORM_WKDM_ALIGNMENT_MASK__
#define C_SEG_OFFSET_ALIGNMENT_BOUNDARY __PLATFORM_WKDM_ALIGNMENT_BOUNDARY__
#endif

#define C_SEG_SHOULD_MINORCOMPACT_NOW(cseg)     ((C_SEG_UNUSED_BYTES(cseg) >= (C_SEG_BUFSIZE / 4)) ? 1 : 0)

/*
 * the decsion to force a c_seg to be major compacted is based on 2 criteria
 * 1) is the c_seg buffer almost empty (i.e. we have a chance to merge it with another c_seg)
 * 2) are there at least a minimum number of slots unoccupied so that we have a chance
 *    of combining this c_seg with another one.
 */
#define C_SEG_SHOULD_MAJORCOMPACT_NOW(cseg)                                                                                     \
	((((cseg->c_bytes_unused + (C_SEG_BUFSIZE - C_SEG_OFFSET_TO_BYTES(c_seg->c_nextoffset))) >= (C_SEG_BUFSIZE / 8)) &&     \
	  ((C_SLOT_MAX_INDEX - cseg->c_slots_used) > (C_SEG_BUFSIZE / PAGE_SIZE))) \
	? 1 : 0)

#define C_SEG_ONDISK_IS_SPARSE(cseg)    ((cseg->c_bytes_used < cseg->c_bytes_unused) ? 1 : 0)
#define C_SEG_IS_ONDISK(cseg)           ((cseg->c_state == C_ON_SWAPPEDOUT_Q || cseg->c_state == C_ON_SWAPPEDOUTSPARSE_Q))
#define C_SEG_IS_ON_DISK_OR_SOQ(cseg)   ((cseg->c_state == C_ON_SWAPPEDOUT_Q || \
	                                  cseg->c_state == C_ON_SWAPPEDOUTSPARSE_Q || \
	                                  cseg->c_state == C_ON_SWAPOUT_Q || \
	                                  cseg->c_state == C_ON_SWAPIO_Q))


#define C_SEG_WAKEUP_DONE(cseg)                         \
	MACRO_BEGIN                                     \
	assert((cseg)->c_busy);                         \
	(cseg)->c_busy = 0;                             \
	assert((cseg)->c_busy_for_thread != NULL);      \
	assert((((cseg)->c_busy_for_thread = NULL), TRUE));     \
	if ((cseg)->c_wanted) {                         \
	        (cseg)->c_wanted = 0;                   \
	        thread_wakeup((event_t) (cseg));        \
	}                                               \
	MACRO_END

#define C_SEG_BUSY(cseg)                                \
	MACRO_BEGIN                                     \
	assert((cseg)->c_busy == 0);                    \
	(cseg)->c_busy = 1;                             \
	assert((cseg)->c_busy_for_thread == NULL);      \
	assert((((cseg)->c_busy_for_thread = current_thread()), TRUE)); \
	MACRO_END


extern vm_map_t compressor_map;

#if DEVELOPMENT || DEBUG
extern boolean_t write_protect_c_segs;
extern int vm_compressor_test_seg_wp;

#define C_SEG_MAKE_WRITEABLE(cseg)                      \
	MACRO_BEGIN                                     \
	if (write_protect_c_segs) {                     \
	        vm_map_protect(compressor_map,                  \
	                       (vm_map_offset_t)cseg->c_store.c_buffer,         \
	                       (vm_map_offset_t)&cseg->c_store.c_buffer[C_SEG_BYTES_TO_OFFSET(C_SEG_ALLOCSIZE)],\
	                       VM_PROT_READ | VM_PROT_WRITE,    \
	                       0);                              \
	}                               \
	MACRO_END

#define C_SEG_WRITE_PROTECT(cseg)                       \
	MACRO_BEGIN                                     \
	if (write_protect_c_segs) {                     \
	        vm_map_protect(compressor_map,                  \
	                       (vm_map_offset_t)cseg->c_store.c_buffer,         \
	                       (vm_map_offset_t)&cseg->c_store.c_buffer[C_SEG_BYTES_TO_OFFSET(C_SEG_ALLOCSIZE)],\
	                       VM_PROT_READ,                    \
	                       0);                              \
	}                                                       \
	if (vm_compressor_test_seg_wp) {                                \
	        volatile uint32_t vmtstmp = *(volatile uint32_t *)cseg->c_store.c_buffer; \
	        *(volatile uint32_t *)cseg->c_store.c_buffer = 0xDEADABCD; \
	        (void) vmtstmp;                                         \
	}                                                               \
	MACRO_END
#endif

typedef struct c_segment *c_segment_t;
typedef struct c_slot   *c_slot_t;

uint64_t vm_compressor_total_compressions(void);
void vm_wake_compactor_swapper(void);
void vm_run_compactor(void);
void vm_thrashing_jetsam_done(void);
void vm_consider_waking_compactor_swapper(void);
void vm_consider_swapping(void);
void vm_compressor_flush(void);
void c_seg_free(c_segment_t);
void c_seg_free_locked(c_segment_t);
void c_seg_insert_into_age_q(c_segment_t);
void c_seg_need_delayed_compaction(c_segment_t, boolean_t);

void vm_decompressor_lock(void);
void vm_decompressor_unlock(void);

void vm_compressor_delay_trim(void);
void vm_compressor_do_warmup(void);
void vm_compressor_record_warmup_start(void);
void vm_compressor_record_warmup_end(void);

int                     vm_wants_task_throttled(task_t);

extern void             vm_compaction_swapper_do_init(void);
extern void             vm_compressor_swap_init(void);
extern void             vm_compressor_init_locks(void);
extern lck_rw_t         c_master_lock;

#if ENCRYPTED_SWAP
extern void             vm_swap_decrypt(c_segment_t);
#endif /* ENCRYPTED_SWAP */

extern int              vm_swap_low_on_space(void);
extern kern_return_t    vm_swap_get(c_segment_t, uint64_t, uint64_t);
extern void             vm_swap_free(uint64_t);
extern void             vm_swap_consider_defragmenting(int);

extern void             c_seg_swapin_requeue(c_segment_t, boolean_t, boolean_t, boolean_t);
extern int              c_seg_swapin(c_segment_t, boolean_t, boolean_t);
extern void             c_seg_wait_on_busy(c_segment_t);
extern void             c_seg_trim_tail(c_segment_t);
extern void             c_seg_switch_state(c_segment_t, int, boolean_t);

extern boolean_t        fastwake_recording_in_progress;
extern int              compaction_swapper_inited;
extern int              compaction_swapper_running;
extern uint64_t         vm_swap_put_failures;

extern int              c_overage_swapped_count;
extern int              c_overage_swapped_limit;

extern queue_head_t     c_minor_list_head;
extern queue_head_t     c_age_list_head;
extern queue_head_t     c_swapout_list_head;
extern queue_head_t     c_swappedout_list_head;
extern queue_head_t     c_swappedout_sparse_list_head;

extern uint32_t         c_age_count;
extern uint32_t         c_swapout_count;
extern uint32_t         c_swappedout_count;
extern uint32_t         c_swappedout_sparse_count;

extern int64_t          compressor_bytes_used;
extern uint64_t         first_c_segment_to_warm_generation_id;
extern uint64_t         last_c_segment_to_warm_generation_id;
extern boolean_t        hibernate_flushing;
extern boolean_t        hibernate_no_swapspace;
extern boolean_t        hibernate_in_progress_with_pinned_swap;
extern uint32_t         swapout_target_age;

extern void c_seg_insert_into_q(queue_head_t *, c_segment_t);

extern uint32_t vm_compressor_minorcompact_threshold_divisor;
extern uint32_t vm_compressor_majorcompact_threshold_divisor;
extern uint32_t vm_compressor_unthrottle_threshold_divisor;
extern uint32_t vm_compressor_catchup_threshold_divisor;

extern uint32_t vm_compressor_minorcompact_threshold_divisor_overridden;
extern uint32_t vm_compressor_majorcompact_threshold_divisor_overridden;
extern uint32_t vm_compressor_unthrottle_threshold_divisor_overridden;
extern uint32_t vm_compressor_catchup_threshold_divisor_overridden;

extern uint64_t vm_compressor_compute_elapsed_msecs(clock_sec_t, clock_nsec_t, clock_sec_t, clock_nsec_t);

#define PAGE_REPLACEMENT_DISALLOWED(enable)     (enable == TRUE ? lck_rw_lock_shared(&c_master_lock) : lck_rw_done(&c_master_lock))
#define PAGE_REPLACEMENT_ALLOWED(enable)        (enable == TRUE ? lck_rw_lock_exclusive(&c_master_lock) : lck_rw_done(&c_master_lock))


#define AVAILABLE_NON_COMPRESSED_MEMORY         (vm_page_active_count + vm_page_inactive_count + vm_page_free_count + vm_page_speculative_count)
#define AVAILABLE_MEMORY                        (AVAILABLE_NON_COMPRESSED_MEMORY + VM_PAGE_COMPRESSOR_COUNT)

/*
 * TODO, there may be a minor optimisation opportunity to replace these divisions
 * with multiplies and shifts
 *
 * By multiplying by 10, the divisors can have more precision w/o resorting to floating point... a divisor specified as 25 is in reality a divide by 2.5
 * By multiplying by 9, you get a number ~11% smaller which allows us to have another limit point derived from the same base
 * By multiplying by 11, you get a number ~10% bigger which allows us to generate a reset limit derived from the same base which is useful for hysteresis
 */

#define VM_PAGE_COMPRESSOR_COMPACT_THRESHOLD            (((AVAILABLE_MEMORY) * 10) / (vm_compressor_minorcompact_threshold_divisor ? vm_compressor_minorcompact_threshold_divisor : 10))
#define VM_PAGE_COMPRESSOR_SWAP_THRESHOLD               (((AVAILABLE_MEMORY) * 10) / (vm_compressor_majorcompact_threshold_divisor ? vm_compressor_majorcompact_threshold_divisor : 10))

#define VM_PAGE_COMPRESSOR_SWAP_UNTHROTTLE_THRESHOLD    (((AVAILABLE_MEMORY) * 10) / (vm_compressor_unthrottle_threshold_divisor ? vm_compressor_unthrottle_threshold_divisor : 10))
#define VM_PAGE_COMPRESSOR_SWAP_RETHROTTLE_THRESHOLD    (((AVAILABLE_MEMORY) * 11) / (vm_compressor_unthrottle_threshold_divisor ? vm_compressor_unthrottle_threshold_divisor : 11))

#define VM_PAGE_COMPRESSOR_SWAP_HAS_CAUGHTUP_THRESHOLD  (((AVAILABLE_MEMORY) * 11) / (vm_compressor_catchup_threshold_divisor ? vm_compressor_catchup_threshold_divisor : 11))
#define VM_PAGE_COMPRESSOR_SWAP_CATCHUP_THRESHOLD       (((AVAILABLE_MEMORY) * 10) / (vm_compressor_catchup_threshold_divisor ? vm_compressor_catchup_threshold_divisor : 10))
#define VM_PAGE_COMPRESSOR_HARD_THROTTLE_THRESHOLD      (((AVAILABLE_MEMORY) * 9) / (vm_compressor_catchup_threshold_divisor ? vm_compressor_catchup_threshold_divisor : 9))

#ifdef  CONFIG_EMBEDDED
#define AVAILABLE_NON_COMPRESSED_MIN                    20000
#define COMPRESSOR_NEEDS_TO_SWAP()              (((AVAILABLE_NON_COMPRESSED_MEMORY < VM_PAGE_COMPRESSOR_SWAP_THRESHOLD) || \
	                                          (AVAILABLE_NON_COMPRESSED_MEMORY < AVAILABLE_NON_COMPRESSED_MIN)) ? 1 : 0)
#else
#define COMPRESSOR_NEEDS_TO_SWAP()              ((AVAILABLE_NON_COMPRESSED_MEMORY < VM_PAGE_COMPRESSOR_SWAP_THRESHOLD) ? 1 : 0)
#endif

#define HARD_THROTTLE_LIMIT_REACHED()           ((AVAILABLE_NON_COMPRESSED_MEMORY < VM_PAGE_COMPRESSOR_HARD_THROTTLE_THRESHOLD) ? 1 : 0)
#define SWAPPER_NEEDS_TO_UNTHROTTLE()           ((AVAILABLE_NON_COMPRESSED_MEMORY < VM_PAGE_COMPRESSOR_SWAP_UNTHROTTLE_THRESHOLD) ? 1 : 0)
#define SWAPPER_NEEDS_TO_RETHROTTLE()           ((AVAILABLE_NON_COMPRESSED_MEMORY > VM_PAGE_COMPRESSOR_SWAP_RETHROTTLE_THRESHOLD) ? 1 : 0)
#define SWAPPER_NEEDS_TO_CATCHUP()              ((AVAILABLE_NON_COMPRESSED_MEMORY < VM_PAGE_COMPRESSOR_SWAP_CATCHUP_THRESHOLD) ? 1 : 0)
#define SWAPPER_HAS_CAUGHTUP()                  ((AVAILABLE_NON_COMPRESSED_MEMORY > VM_PAGE_COMPRESSOR_SWAP_HAS_CAUGHTUP_THRESHOLD) ? 1 : 0)
#define COMPRESSOR_NEEDS_TO_MINOR_COMPACT()     ((AVAILABLE_NON_COMPRESSED_MEMORY < VM_PAGE_COMPRESSOR_COMPACT_THRESHOLD) ? 1 : 0)


#ifdef  CONFIG_EMBEDDED
#define COMPRESSOR_FREE_RESERVED_LIMIT          28
#else
#define COMPRESSOR_FREE_RESERVED_LIMIT          128
#endif

uint32_t vm_compressor_get_encode_scratch_size(void);
uint32_t vm_compressor_get_decode_scratch_size(void);

#define COMPRESSOR_SCRATCH_BUF_SIZE vm_compressor_get_encode_scratch_size()

#if RECORD_THE_COMPRESSED_DATA
extern void      c_compressed_record_init(void);
extern void      c_compressed_record_write(char *, int);
#endif

extern lck_mtx_t        *c_list_lock;

#if DEVELOPMENT || DEBUG
extern uint32_t vm_ktrace_enabled;

#define VMKDBG(x, ...)          \
MACRO_BEGIN                     \
if (vm_ktrace_enabled) {        \
	KDBG(x, ## __VA_ARGS__);\
}                               \
MACRO_END
#endif
