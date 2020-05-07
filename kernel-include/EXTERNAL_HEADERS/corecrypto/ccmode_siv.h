/*
 *  ccmode_siv.h
 *  corecrypto
 *
 *  Created on 11/13/2015
 *
 *  Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCMODE_SIV_H_
#define _CORECRYPTO_CCMODE_SIV_H_

#include <corecrypto/cc.h>
#include <corecrypto/ccmode.h>
#include <corecrypto/ccmode_impl.h>

#include <corecrypto/cccmac.h>

/* This provide an implementation of SIV
 as specified in https://tools.ietf.org/html/rfc5297
 also in http://csrc.nist.gov/groups/ST/toolkit/BCM/documents/proposedmodes/siv/siv.pdf
 Counter Mode where IV is based on CMAC
 */

cc_aligned_struct(16) ccsiv_ctx;

struct ccmode_siv {
    size_t size;        /* first argument to ccsiv_ctx_decl(). */
    size_t block_size;
    int (*init)(const struct ccmode_siv *siv, ccsiv_ctx *ctx,
                 size_t key_len, const uint8_t *key);
    int (*set_nonce)(ccsiv_ctx *ctx,  size_t nbytes, const uint8_t *in);  // could just be ccm with NULL out
    int (*auth)(ccsiv_ctx *ctx,  size_t nbytes, const uint8_t *in);  // could just be ccm with NULL out
    int (*crypt)(ccsiv_ctx *ctx, size_t nbytes, const uint8_t *in, uint8_t *out);
    int (*reset)(ccsiv_ctx *ctx);
    const struct ccmode_cbc *cbc;
    const struct ccmode_ctr *ctr;
};

#define ccsiv_ctx_decl(_size_, _name_)  cc_ctx_decl(ccsiv_ctx, _size_, _name_)
#define ccsiv_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

// Functions

CC_INLINE size_t ccsiv_context_size(const struct ccmode_siv *mode)
{
    return mode->size;
}

CC_INLINE size_t ccsiv_block_size(const struct ccmode_siv *mode)
{
    return mode->block_size;
}

CC_INLINE size_t ccsiv_ciphertext_size(const struct ccmode_siv *mode,
                                       size_t plaintext_size)
{
    return plaintext_size + mode->cbc->block_size;
}

CC_INLINE size_t ccsiv_plaintext_size(const struct ccmode_siv *mode,
                                       size_t ciphertext_size)
{
    if (ciphertext_size<mode->cbc->block_size) {
        return 0; // error
    }
    return ciphertext_size - mode->cbc->block_size;
}

// Supported key sizes are 32, 48, 64 bytes
CC_INLINE int ccsiv_init(const struct ccmode_siv *mode, ccsiv_ctx *ctx,
                          size_t key_byte_len, const uint8_t *key)
{
    return mode->init(mode, ctx, key_byte_len, key);
}

// Process nonce. it is actually just an authenticated data
CC_INLINE int ccsiv_set_nonce(const struct ccmode_siv *mode, ccsiv_ctx *ctx,
                         size_t nbytes, const uint8_t *in)
{
    return mode->set_nonce(ctx, nbytes, in);
}

// Process authenticated data. Taken into account for authentication but not
// encrypted
CC_INLINE int ccsiv_aad(const struct ccmode_siv *mode, ccsiv_ctx *ctx,
                            size_t nbytes, const uint8_t *in)
{
    return mode->auth(ctx, nbytes, in);
}

// Encryption data. Authenticated and encrypted.
// Encrypt/Decrypt can only be called once
CC_INLINE int ccsiv_crypt(const struct ccmode_siv *mode, ccsiv_ctx *ctx,
                            size_t nbytes, const uint8_t *in, uint8_t *out)
{
    return mode->crypt(ctx, nbytes, in, out);
}

// Clear all context for reuse.
CC_INLINE int ccsiv_reset(const struct ccmode_siv *mode, ccsiv_ctx *ctx)
{
    return mode->reset(ctx);
}

// One shot with only one vector of adata
CC_INLINE int ccsiv_one_shot(const struct ccmode_siv *mode,
                              size_t key_len, const uint8_t *key,
                              unsigned nonce_nbytes, const uint8_t* nonce,
                              unsigned adata_nbytes, const uint8_t* adata,
                              size_t in_nbytes, const uint8_t *in, uint8_t *out)
{
    int rc;
    ccsiv_ctx_decl(mode->size, ctx);
    rc=mode->init(mode, ctx, key_len, key);
    if (rc) {return rc;}
    rc=mode->set_nonce(ctx, nonce_nbytes, nonce);
    if (rc) {return rc;}
    rc=mode->auth(ctx, adata_nbytes, adata);
    if (rc) {return rc;}
    rc=mode->crypt(ctx, in_nbytes, in, out);
    if (rc) {return rc;}
    ccsiv_ctx_clear(mode->size, ctx);
    return rc;
}

#endif /* _CORECRYPTO_CCMODE_H_ */
