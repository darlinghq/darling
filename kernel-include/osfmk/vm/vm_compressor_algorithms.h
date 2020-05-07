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

#pragma once

#if XNU_KERNEL_PRIVATE
//DRKTODO: the decompression side stats should be either made optional or
//per-CPU to avoid cacheline contention

typedef struct {
	uint64_t lz4_compressions;
	uint64_t lz4_compression_failures;
	uint64_t lz4_compressed_bytes;
	uint64_t lz4_wk_compression_delta;
	uint64_t lz4_wk_compression_negative_delta;
	uint64_t lz4_post_wk_compressions;

	uint64_t wk_compressions;
	uint64_t wk_cabstime;
	uint64_t wk_sv_compressions;
	uint64_t wk_mzv_compressions;
	uint64_t wk_compression_failures;
	uint64_t wk_compressed_bytes_total;
	uint64_t wk_compressions_exclusive;
	uint64_t wk_compressed_bytes_exclusive;

	uint64_t wkh_compressions;
	uint64_t wkh_cabstime;
	uint64_t wks_compressions;
	uint64_t wks_cabstime;
	uint64_t wks_compressed_bytes;
	uint64_t wks_compression_failures;
	uint64_t wks_sv_compressions;

	uint64_t lz4_decompressions;
	uint64_t lz4_decompressed_bytes;
	uint64_t uc_decompressions;

	uint64_t wk_decompressions;
	uint64_t wk_dabstime;

	uint64_t wkh_decompressions;
	uint64_t wkh_dabstime;

	uint64_t wks_decompressions;
	uint64_t wks_dabstime;

	uint64_t wk_decompressed_bytes;
	uint64_t wk_sv_decompressions;
} compressor_stats_t;

extern compressor_stats_t compressor_stats;

typedef struct {
	uint32_t lz4_selection_max;
	int32_t wkdm_reeval_threshold;
	int32_t lz4_threshold;
	uint32_t lz4_max_failure_skips;
	uint32_t lz4_max_failure_run_length;
	uint32_t lz4_max_preselects;
	uint32_t lz4_run_preselection_threshold;
	uint32_t lz4_run_continue_bytes;
	uint32_t lz4_profitable_bytes;
} compressor_tuneables_t;

extern compressor_tuneables_t vmctune;

int metacompressor(const uint8_t *in, uint8_t *cdst, int32_t outbufsz, uint16_t *codec, void *cscratch, boolean_t *);
void metadecompressor(const uint8_t *source, uint8_t *dest, uint32_t csize, uint16_t ccodec, void *compressor_dscratch);

typedef enum {
	CCWK = 0, // must be 0 or 1
	CCLZ4 = 1, //must be 0 or 1
	CINVALID = 0xFFFF
} vm_compressor_codec_t;

typedef enum {
	CMODE_WK = 0,
	CMODE_LZ4 = 1,
	CMODE_HYB = 2,
	VM_COMPRESSOR_DEFAULT_CODEC = 3,
	CMODE_INVALID = 4
} vm_compressor_mode_t;

void vm_compressor_algorithm_init(void);
int vm_compressor_algorithm(void);
#endif /* XNU_KERNEL_PRIVATE */
