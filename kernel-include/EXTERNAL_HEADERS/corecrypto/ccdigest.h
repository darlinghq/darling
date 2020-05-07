/*
 *  ccdigest.h
 *  corecrypto
 *
 *  Created on 11/30/2010
 *
 *  Copyright (c) 2010,2011,2012,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCDIGEST_H_
#define _CORECRYPTO_CCDIGEST_H_

#include <corecrypto/cc.h>
#include <corecrypto/ccn.h>

/* To malloc a digest context for a given di, use malloc(ccdigest_di_size(di))
   and assign the result to a pointer to a struct ccdigest_ctx. */
struct ccdigest_ctx {
    union {
        uint8_t u8;
        uint32_t u32;
        uint64_t u64;
        cc_unit ccn;
    } state;
} CC_ALIGNED(8);

typedef struct ccdigest_ctx *ccdigest_ctx_t ;

struct ccdigest_state {
    union {
        uint8_t u8;
        uint32_t u32;
        uint64_t u64;
        cc_unit ccn;
    } state;
} CC_ALIGNED(8);

typedef struct ccdigest_state *ccdigest_state_t;

struct ccdigest_info {
    size_t output_size;
    size_t state_size;
    size_t block_size;
    size_t oid_size;
    const unsigned char *oid;
    const void *initial_state;
    void(*compress)(ccdigest_state_t state, size_t nblocks,
                    const void *data);
    void(*final)(const struct ccdigest_info *di, ccdigest_ctx_t ctx,
                 unsigned char *digest);
};

/* Return sizeof a ccdigest_ctx for a given size_t _state_size_ and
   size_t _block_size_. */
#define ccdigest_ctx_size(_state_size_, _block_size_)  ((_state_size_) + sizeof(uint64_t) + (_block_size_) + sizeof(unsigned int))
/* Return sizeof a ccdigest_ctx for a given struct ccdigest_info *_di_. */
#define ccdigest_di_size(_di_)  (ccdigest_ctx_size((_di_)->state_size, (_di_)->block_size))

/* Declare a ccdigest_ctx for a given size_t _state_size_ and
   size_t _block_size_, named _name_.  Can be used in structs or on the
   stack. */
#define ccdigest_ctx_decl(_state_size_, _block_size_, _name_)  cc_ctx_decl(struct ccdigest_ctx, ccdigest_ctx_size(_state_size_, _block_size_), _name_)
#define ccdigest_ctx_clear(_state_size_, _block_size_, _name_) cc_clear(ccdigest_ctx_size(_state_size_, _block_size_), _name_)
/* Declare a ccdigest_ctx for a given size_t _state_size_ and
   size_t _block_size_, named _name_.  Can be used on the stack. */
#define ccdigest_di_decl(_di_, _name_)  cc_ctx_decl(struct ccdigest_ctx, ccdigest_di_size(_di_), _name_)
#define ccdigest_di_clear(_di_, _name_) cc_clear(ccdigest_di_size(_di_), _name_)

/* Digest context field accessors.  Consider the implementation private. */
#define ccdigest_state(_di_, _ctx_)      ((struct ccdigest_state *)(&((ccdigest_ctx_t)(_ctx_))->state.u8 + sizeof(uint64_t)))

#define ccdigest_state_u8(_di_, _ctx_)   ccdigest_u8(ccdigest_state((_di_), (_ctx_)))
#define ccdigest_state_u32(_di_, _ctx_)  ccdigest_u32(ccdigest_state((_di_), (_ctx_)))
#define ccdigest_state_u64(_di_, _ctx_)  ccdigest_u64(ccdigest_state((_di_), (_ctx_)))
#define ccdigest_state_ccn(_di_, _ctx_)  ccdigest_ccn(ccdigest_state((_di_), (_ctx_)))

#define ccdigest_nbits(_di_, _ctx_)      (((uint64_t *)(&((ccdigest_ctx_t)(_ctx_))->state.u8))[0])
#define ccdigest_data(_di_, _ctx_)       (&((ccdigest_ctx_t)(_ctx_))->state.u8 + (_di_)->state_size + sizeof(uint64_t))
#define ccdigest_num(_di_, _ctx_)        (((unsigned int *)(&((ccdigest_ctx_t)(_ctx_))->state.u8 + (_di_)->state_size + sizeof(uint64_t) + (_di_)->block_size))[0])

/* Digest state field accessors.  Consider the implementation private. */
#define ccdigest_u8(_state_)             (&((ccdigest_state_t)(_state_))->state.u8)
#define ccdigest_u32(_state_)            (&((ccdigest_state_t)(_state_))->state.u32)
#define ccdigest_u64(_state_)            (&((ccdigest_state_t)(_state_))->state.u64)
#define ccdigest_ccn(_state_)            (&((ccdigest_state_t)(_state_))->state.ccn)

void ccdigest_init(const struct ccdigest_info *di, ccdigest_ctx_t ctx);
void ccdigest_update(const struct ccdigest_info *di, ccdigest_ctx_t ctx,
                     size_t len, const void *data);

CC_INLINE
void ccdigest_final(const struct ccdigest_info *di, ccdigest_ctx_t ctx, unsigned char *digest)
{
    di->final(di,ctx,digest);
}

void ccdigest(const struct ccdigest_info *di, size_t len,
              const void *data, void *digest);

#define OID_DEF(_VALUE_)  ((const unsigned char *)_VALUE_)

#define CC_DIGEST_OID_MD2       OID_DEF("\x06\x08\x2A\x86\x48\x86\xF7\x0D\x02\x02")
#define CC_DIGEST_OID_MD4       OID_DEF("\x06\x08\x2A\x86\x48\x86\xF7\x0D\x02\x04")
#define CC_DIGEST_OID_MD5       OID_DEF("\x06\x08\x2A\x86\x48\x86\xF7\x0D\x02\x05")
#define CC_DIGEST_OID_SHA1      OID_DEF("\x06\x05\x2b\x0e\x03\x02\x1a")
#define CC_DIGEST_OID_SHA224    OID_DEF("\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x04")
#define CC_DIGEST_OID_SHA256    OID_DEF("\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01")
#define CC_DIGEST_OID_SHA384    OID_DEF("\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x02")
#define CC_DIGEST_OID_SHA512    OID_DEF("\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x03")
#define CC_DIGEST_OID_RMD160    OID_DEF("\x06\x05\x2B\x24\x03\x02\x01")

#endif /* _CORECRYPTO_CCDIGEST_H_ */
