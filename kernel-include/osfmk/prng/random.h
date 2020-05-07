/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

#ifndef _PRNG_RANDOM_H_
#define _PRNG_RANDOM_H_

__BEGIN_DECLS

#include <corecrypto/cckprng.h>

#ifdef XNU_KERNEL_PRIVATE

#define ENTROPY_BUFFER_BYTE_SIZE 32

#define ENTROPY_BUFFER_SIZE (ENTROPY_BUFFER_BYTE_SIZE / sizeof(uint32_t))

// This mask can be applied to EntropyData.sample_count to get an
// index suitable for storing the next sample in
// EntropyData.buffer. Note that ENTROPY_BUFFER_SIZE must be a power
// of two for the following mask calculation to be valid.
#define ENTROPY_BUFFER_INDEX_MASK (ENTROPY_BUFFER_SIZE - 1)

typedef struct entropy_data {
	/*
	 * TODO: Should sample_count be volatile?  Are we exposed to any races that
	 * we care about if it is not?
	 */

	// At 32 bits, this counter can overflow. Since we're primarily
	// interested in the delta from one read to the next, we don't
	// worry about this too much.
	uint32_t sample_count;
	uint32_t buffer[ENTROPY_BUFFER_SIZE];
} entropy_data_t;

extern entropy_data_t EntropyData;

/* Trace codes for DBG_SEC_KERNEL: */
#define ENTROPY_READ(n) SECURITYDBG_CODE(DBG_SEC_KERNEL, n) /* n: 0 .. 3 */

void random_cpu_init(int cpu);


#endif /* XNU_KERNEL_PRIVATE */

void register_and_init_prng(struct cckprng_ctx *ctx, const struct cckprng_funcs *funcs);

#include <kern/simple_lock.h>
/* Definitions for boolean PRNG */
#define RANDOM_BOOL_GEN_SEED_COUNT 4
struct bool_gen {
	unsigned int seed[RANDOM_BOOL_GEN_SEED_COUNT];
	unsigned int state;
	decl_simple_lock_data(, lock);
};

extern void random_bool_init(struct bool_gen * bg);

extern void random_bool_gen_entropy(struct bool_gen * bg, unsigned int * buffer, int count);

extern unsigned int random_bool_gen_bits(struct bool_gen * bg, unsigned int * buffer, unsigned int count, unsigned int numbits);

__END_DECLS

#endif /* _PRNG_RANDOM_H_ */
