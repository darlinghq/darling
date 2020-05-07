/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __FIREHOSE_CHUNK_PRIVATE__
#define __FIREHOSE_CHUNK_PRIVATE__

#if KERNEL
#include <machine/atomic.h>
#endif
#include <stdatomic.h>
#include <sys/param.h>
#include <os/base.h>
#include "firehose_types_private.h"
#include "tracepoint_private.h"

__BEGIN_DECLS

#define FIREHOSE_CHUNK_SIZE                     4096ul

#define FIREHOSE_CHUNK_POS_ENTRY_OFFS_INC       (1ULL <<  0)
#define FIREHOSE_CHUNK_POS_PRIVATE_OFFS_INC     (1ULL << 16)
#define FIREHOSE_CHUNK_POS_REFCNT_INC           (1ULL << 32)
#define FIREHOSE_CHUNK_POS_FULL_BIT             (1ULL << 56)
#define FIREHOSE_CHUNK_POS_USABLE_FOR_STREAM(pos, stream) \
	        ((((pos).fcp_pos >> 48) & 0x1ff) == (uint16_t)stream)

typedef union {
	_Atomic(uint64_t) fcp_atomic_pos;
	uint64_t fcp_pos;
	struct {
		uint16_t fcp_next_entry_offs;
		uint16_t fcp_private_offs;
		uint8_t  fcp_refcnt;
		uint8_t  fcp_qos;
		uint8_t  fcp_stream;
		uint8_t  fcp_flag_full : 1;
		uint8_t  fcp_flag_io : 1;
		uint8_t  fcp_quarantined : 1;
		uint8_t  _fcp_flag_unused : 5;
	};
} firehose_chunk_pos_u;

typedef struct firehose_chunk_s {
	uint8_t  fc_start[0];
	firehose_chunk_pos_u fc_pos;
	uint64_t fc_timestamp;
	uint8_t  fc_data[FIREHOSE_CHUNK_SIZE - 8 - 8];
} *firehose_chunk_t;

typedef struct firehose_chunk_range_s {
	uint16_t fcr_offset; // offset from the start of the chunk
	uint16_t fcr_length;
} *firehose_chunk_range_t;

#if defined(KERNEL) || defined(OS_FIREHOSE_SPI)

OS_ALWAYS_INLINE
static inline bool
firehose_chunk_pos_fits(firehose_chunk_pos_u pos, uint16_t size)
{
	return pos.fcp_next_entry_offs + size <= pos.fcp_private_offs;
}

#define FIREHOSE_CHUNK_TRY_RESERVE_FAIL_ENQUEUE  (-1)
#define FIREHOSE_CHUNK_TRY_RESERVE_FAIL          ( 0)

OS_ALWAYS_INLINE
static inline long
firehose_chunk_tracepoint_try_reserve(firehose_chunk_t fc, uint64_t stamp,
    firehose_stream_t stream, uint8_t qos, uint16_t pubsize,
    uint16_t privsize, uint8_t **privptr)
{
	const uint16_t ft_size = offsetof(struct firehose_tracepoint_s, ft_data);
	firehose_chunk_pos_u orig, pos;
	bool reservation_failed, stamp_delta_fits;

	stamp_delta_fits = ((stamp - fc->fc_timestamp) >> 48) == 0;

	// no acquire barrier because the returned space is written to only
	os_atomic_rmw_loop(&fc->fc_pos.fcp_atomic_pos,
	    orig.fcp_pos, pos.fcp_pos, relaxed, {
		if (orig.fcp_pos == 0) {
		        // we acquired a really really old reference, and we probably
		        // just faulted in a new page
		        os_atomic_rmw_loop_give_up(return FIREHOSE_CHUNK_TRY_RESERVE_FAIL);
		}
		if (!FIREHOSE_CHUNK_POS_USABLE_FOR_STREAM(orig, stream)) {
		        // nothing to do if the chunk is full, or the stream doesn't match,
		        // in which case the thread probably:
		        // - loaded the chunk ref
		        // - been suspended a long while
		        // - read the chunk to find a very old thing
		        os_atomic_rmw_loop_give_up(return FIREHOSE_CHUNK_TRY_RESERVE_FAIL);
		}
		pos = orig;
		if (!firehose_chunk_pos_fits(orig,
		ft_size + pubsize + privsize) || !stamp_delta_fits) {
		        pos.fcp_flag_full = true;
		        reservation_failed = true;
		} else {
		        if (qos > pos.fcp_qos) {
		                pos.fcp_qos = qos;
			}
		        // using these *_INC macros is so that the compiler generates better
		        // assembly: using the struct individual fields forces the compiler
		        // to handle carry propagations, and we know it won't happen
		        pos.fcp_pos += roundup(ft_size + pubsize, 8) *
		        FIREHOSE_CHUNK_POS_ENTRY_OFFS_INC;
		        pos.fcp_pos -= privsize * FIREHOSE_CHUNK_POS_PRIVATE_OFFS_INC;
		        pos.fcp_pos += FIREHOSE_CHUNK_POS_REFCNT_INC;
		        const uint16_t minimum_payload_size = 16;
		        if (!firehose_chunk_pos_fits(pos,
		        roundup(ft_size + minimum_payload_size, 8))) {
		                // if we can't even have minimum_payload_size bytes of payload
		                // for the next tracepoint, just flush right away
		                pos.fcp_flag_full = true;
			}
		        reservation_failed = false;
		}
	});

	if (reservation_failed) {
		if (pos.fcp_refcnt) {
			// nothing to do, there is a thread writing that will pick up
			// the "FULL" flag on flush and push as a consequence
			return FIREHOSE_CHUNK_TRY_RESERVE_FAIL;
		}
		// caller must enqueue chunk
		return FIREHOSE_CHUNK_TRY_RESERVE_FAIL_ENQUEUE;
	}
	if (privptr) {
		*privptr = (uint8_t *)((uintptr_t)fc->fc_start + pos.fcp_private_offs);
	}
	return orig.fcp_next_entry_offs;
}

OS_ALWAYS_INLINE
static inline firehose_tracepoint_t
firehose_chunk_tracepoint_begin(firehose_chunk_t fc, uint64_t stamp,
    uint16_t pubsize, uint64_t thread_id, long offset)
{
	firehose_tracepoint_t ft = (firehose_tracepoint_t)
	    __builtin_assume_aligned((void *)((uintptr_t)fc->fc_start + (uintptr_t)offset), 8);
	stamp -= fc->fc_timestamp;
	stamp |= (uint64_t)pubsize << 48;
	// The compiler barrier is needed for userland process death handling, see
	// (tracepoint-begin) in libdispatch's firehose_buffer_stream_chunk_install.
	atomic_store_explicit(&ft->ft_atomic_stamp_and_length, stamp,
	    memory_order_relaxed);
	__asm__ __volatile__ ("" ::: "memory");
	ft->ft_thread = thread_id;
	return ft;
}

OS_ALWAYS_INLINE
static inline bool
firehose_chunk_tracepoint_end(firehose_chunk_t fc,
    firehose_tracepoint_t ft, firehose_tracepoint_id_u ftid)
{
	firehose_chunk_pos_u pos;

	atomic_store_explicit(&ft->ft_id.ftid_atomic_value,
	    ftid.ftid_value, memory_order_release);
	pos.fcp_pos = atomic_fetch_sub_explicit(&fc->fc_pos.fcp_atomic_pos,
	    FIREHOSE_CHUNK_POS_REFCNT_INC, memory_order_relaxed);
	return pos.fcp_refcnt == 1 && pos.fcp_flag_full;
}

#endif // defined(KERNEL) || defined(OS_FIREHOSE_SPI)

__END_DECLS

#endif // __FIREHOSE_CHUNK_PRIVATE__
