/*
 *  cckprng.h
 *  corecrypto
 *
 *  Created on 12/7/2017
 *
 *  Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCKPRNG_H_
#define _CORECRYPTO_CCKPRNG_H_

#include <stdbool.h>

#include <corecrypto/cc.h>

#define CCKPRNG_YARROW 0

#if CCKPRNG_YARROW

typedef struct PRNG *PrngRef;

struct cckprng_ctx {
    PrngRef prng;
    uint64_t bytes_since_entropy;
    uint64_t bytes_generated;
};

#define CCKPRNG_ENTROPY_INTERVAL (1 << 14)
#define CCKPRNG_RESEED_NTICKS 50

typedef struct cckprng_ctx *cckprng_ctx_t;

#else

// This is a Fortuna-inspired PRNG. While it differs from Fortuna in
// many minor details, the biggest difference is its support for
// multiple independent output generators. This is to make it suitable
// for use in concurrent environments.
//
// This PRNG targets a 256-bit security level.
//
// First, the user should call cckprng_init. The user must specify the
// maximum number of output generators that might be
// needed. (Typically, users should align this argument with the
// number of available CPUs.)
//
// The user must also provide a read-only handle to an entropy
// source. This is a fixed-size buffer that will receive entropy
// updates out of band from the PRNG (e.g. in an interrupt
// handler). The PRNG will consume entropy from this buffer according
// to an internal schedule driven by calls to cckprng_refresh (see
// below).
//
// The user should call cckprng_initgen for as many output generators
// as are needed. The numeric argument is an identifier to be reused
// during calls to cckprng_generate (see below) and must be less than
// the maximum number of generators specified to cckprng_init.
//
// After initialization, the user is free to call cckprng_generate to
// generate random bytes. The user must specify the generator in this
// call using a numeric identifier passed in the call to
// cckprng_initgen.
//
// Output generation is limited to 256 bytes per request. Users should
// make multiple requests if more output is needed.
//
// The user is expected to call cckprng_refresh regularly. This
// function consumes entropy and mixes it into the output generators
// according to an internal schedule.
//
// This implementation is thread-safe. Internally, a set of mutexes
// guard access to internal state. Most functions rely on a single
// mutex to protect shared state. The main exception is the
// cckprng_generate function, which uses a per-generator mutex to
// allow concurrent output generation on different threads.
//
// Another important exception is cckprng_refresh. While this function
// relies on the shared mutex, it returns immediately if it cannot
// acquire it.
//
// The PRNG also supports user-initiated reseeds. This is to support a
// user-writable random device.
//
// This PRNG supports reseeds concurrent with output generation,
// i.e. it is safe to call cckprng_reseed or cckprng_refresh while
// another thread is calling cckprng_generate.

#define CCKPRNG_NPOOLS 32
#define CCKPRNG_SEED_NBYTES 32
#define CCKPRNG_POOL_NBYTES 32
#define CCKPRNG_KEY_NBYTES 32

struct cckprng_gen_diag {
    // The number of times this generator has been rekeyed from the master seed
    uint64_t nrekeys;

    // The number of requests this generator has fulfilled
    uint64_t out_nreqs;

    // The total number of bytes this generator has generated over all requests
    uint64_t out_nbytes;

    // The maximum number of bytes this generator has generated in any one request
    uint64_t out_nbytes_req_max;

    // The total number of bytes this generator has generated since the last rekey
    uint64_t out_nbytes_key;

    // The maximum total number of bytes this generator has generated between two rekeys
    uint64_t out_nbytes_key_max;
};

struct cckprng_pool_diag {
    // The number of samples currently resident in the pool
    uint64_t nsamples;

    // The number of times this pool has been drained in a reseed
    uint64_t ndrains;

    // The maximum number of samples this pool has held at any one time
    uint64_t nsamples_max;
};

struct cckprng_diag {
    // The number of reseeds via user input (e.g. by writing to /dev/random)
    uint64_t userreseed_nreseeds;

    // The number of reseeds via the scheduler
    uint64_t schedreseed_nreseeds;

    // The maximum number of samples included in any one scheduler reseed
    uint64_t schedreseed_nsamples_max;

    // The maximum number of samples included in any one entropy input
    uint64_t addentropy_nsamples_max;

    // Diagnostics corresponding to individual output generators
    unsigned ngens;
    struct cckprng_gen_diag *gens;

    // Diagnostics corresponding to internal entropy pools
    struct cckprng_pool_diag pools[CCKPRNG_NPOOLS];
};

#if CC_KERNEL

#include <kern/locks.h>

typedef lck_grp_t *cckprng_lock_group;
typedef lck_mtx_t *cckprng_lock_mutex;

struct cckprng_lock_ctx {
    cckprng_lock_group group;
    cckprng_lock_mutex mutex;
};

#else

#include <os/lock.h>

typedef os_unfair_lock cckprng_lock_mutex;

struct cckprng_lock_ctx {
    cckprng_lock_mutex mutex;
};

#endif

struct cckprng_key_ctx {
    uint8_t data[CCKPRNG_KEY_NBYTES];
};

struct cckprng_gen_ctx {
    // We maintain two keys (one live and one idle) to allow
    // concurrent generation and reseeding
    struct cckprng_key_ctx keys[2];
    _Atomic unsigned swap;
    unsigned key_live_idx;
    unsigned key_idle_idx;

    // A counter used in CTR mode
    uint8_t ctr[16];

    // Whether the generator has been initialized
    bool init;

    // A mutex governing this generator's state (but note the idle key
    // context is under control of the PRNG's shared mutex)
    struct {
        cckprng_lock_mutex mutex;
    } lock;
};

struct cckprng_pool_ctx {
    uint8_t data[CCKPRNG_POOL_NBYTES];
};

// This is a handle to an "entropy buffer" to be managed externally
// (i.e. in xnu). This is a non-cryptographic
// accumulator. Practically, the buffer is filled with timestamps
// collected during interrupts. The existing state of the buffer is
// rotated and new timestamps are added in. A counter of raw timing
// samples is also managed externally. The buffer and the counter are
// both subject to data races, which we tolerate.

struct cckprng_entropybuf {

    // A read-only handle to an "entropy buffer" (a non-cryptographic accumulator) to be managed externally
    const void *buf;

    // The size of the entropy buffer
    size_t nbytes;

    // A read-only handle to a count of raw samples in the buffer
    const uint32_t *nsamples;

    // The count of raw samples in the buffer at time of last read
    uint32_t nsamples_last;
};

struct cckprng_sched_ctx {
    // A counter governing the set of entropy pools to drain
    uint64_t reseed_sched;

    // A timestamp from the last reseed
    uint64_t reseed_last;

    // An index used to add entropy to pools in a round-robin style
    unsigned pool_idx;
};

struct cckprng_ctx {

    // The master secret of the PRNG
    uint8_t seed[CCKPRNG_SEED_NBYTES];

    // State used to schedule entropy consumption and reseeds
    struct cckprng_sched_ctx sched;

    // A mutex governing access to shared state
    struct cckprng_lock_ctx lock;

    // The maximum number of generators that may be allocated
    unsigned max_ngens;

    // An array of output generators (allocated dynamically) of length max_ngens
    struct cckprng_gen_ctx *gens;

    // A set of entropy pools
    struct cckprng_pool_ctx pools[CCKPRNG_NPOOLS];

    // A handle to an entropy source managed externally
    struct cckprng_entropybuf entropybuf;

    // Diagnostics for the PRNG
    struct cckprng_diag diag;
};

// This collection of function pointers is just a convenience for
// registering the PRNG with xnu
struct cckprng_funcs {
    void (*init)(struct cckprng_ctx *ctx,
                 unsigned max_ngens,
                 size_t entropybuf_nbytes,
                 const void *entropybuf,
                 const uint32_t *entropybuf_nsamples,
                 size_t seed_nbytes,
                 const void *seed,
                 size_t nonce_nbytes,
                 const void *nonce);
    void (*initgen)(struct cckprng_ctx *ctx, unsigned gen_idx);
    void (*reseed)(struct cckprng_ctx *ctx, size_t nbytes, const void *seed);
    void (*refresh)(struct cckprng_ctx *ctx);
    void (*generate)(struct cckprng_ctx *ctx, unsigned gen_idx, size_t nbytes, void *out);
};

#endif

/*
  @function cckprng_init
  @abstract Initialize a kernel PRNG context.

  @param ctx Context for this instance
  @param max_ngens Maximum count of generators that may be allocated
  @param entropybuf_nbytes Length of the entropy buffer in bytes
  @param entropybuf Read-only pointer to a long-lived entropy buffer
  @param entropybuf_nsamples Read-only pointer to a counter of samples in the entropy buffer
  @param seed_nbytes Length of the seed in bytes
  @param seed Pointer to a high-entropy seed
  @param nonce_nbytes Length of the nonce in bytes
  @param seed Pointer to a single-use nonce

  @discussion @p max_ngens should be set based on an upper bound of CPUs available on the device. The entropy buffer should be managed outside the PRNG and updated continuously (e.g. by an interrupt handler). The count of samples in the entropy buffer needn't be better than a rough estimate.
*/
void cckprng_init(struct cckprng_ctx *ctx,
                  unsigned max_ngens,
                  size_t entropybuf_nbytes,
                  const void *entropybuf,
                  const uint32_t *entropybuf_nsamples,
                  size_t seed_nbytes,
                  const void *seed,
                  size_t nonce_nbytes,
                  const void *nonce);

/*
  @function cckprng_initgen
  @abstract Initialize an output generator.

  @param ctx Context for this instance
  @param gen_idx Index of the generator

  @discussion @p gen_idx must be less than @p max_ngens provided to @cckprng_init and must be unique within the lifetime of a PRNG context. This function will abort if these contracts are violated.
*/
void cckprng_initgen(struct cckprng_ctx *ctx, unsigned gen_idx);

/*
  @function cckprng_reseed
  @abstract Reseed a kernel PRNG context with a user-supplied seed.

  @param ctx Context for this instance
  @param nbytes Length of the seed in bytes
  @param seed Pointer to a high-entropy seed

  @discussion It is safe to expose this function to attacker-controlled requests (e.g. writes to /dev/random).
*/
void cckprng_reseed(struct cckprng_ctx *ctx, size_t nbytes, const void *seed);

/*
  @function cckprng_refresh
  @abstract Consume entropy and reseed according to an internal schedule.

  @param ctx Context for this instance

  @discussion This function should be called on a regular basis. (For example, it is reasonable to call this inline before a call to @p cckprng_generate.) This function will not necessarily consume entropy or reseed the internal state on any given invocation. To force an immediate reseed, call @p cckprng_reseed.
*/
void cckprng_refresh(struct cckprng_ctx *ctx);

#define CCKPRNG_GENERATE_MAX_NBYTES 256

/*
  @function cckprng_generate
  @abstract Generate random values for use in applications.

  @param ctx Context for this instance
  @param gen_idx Index of the output generator
  @param nbytes Length of the desired output in bytes
  @param out Pointer to the output buffer

  @discussion @p gen_idx must be a previous argument to @p cckprng_initgen. @p nbytes must be less than or equal to @p CCKPRNG_GENERATE_MAX_NBYTES. (Callers may invoke this function in a loop to generate larger outputs.) This function will abort if these contracts are violated.
*/
void cckprng_generate(struct cckprng_ctx *ctx, unsigned gen_idx, size_t nbytes, void *out);

#endif /* _CORECRYPTO_CCKPRNG_H_ */
