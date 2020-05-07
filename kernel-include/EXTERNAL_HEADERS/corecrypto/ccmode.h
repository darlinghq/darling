/*
 *  ccmode.h
 *  corecrypto
 *
 *  Created on 12/07/2010
 *
 *  Copyright (c) 2010,2011,2012,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCMODE_H_
#define _CORECRYPTO_CCMODE_H_

#include <corecrypto/cc.h>
#include <corecrypto/ccmode_impl.h>
#include <corecrypto/ccmode_siv.h>
#include <corecrypto/ccmode_siv_hmac.h>

/* ECB mode. */

/* Declare a ecb key named _name_.  Pass the size field of a struct ccmode_ecb
   for _size_. */
#define ccecb_ctx_decl(_size_, _name_) cc_ctx_decl(ccecb_ctx, _size_, _name_)
#define ccecb_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

CC_INLINE size_t ccecb_context_size(const struct ccmode_ecb *mode)
{
    return mode->size;
}

CC_INLINE size_t ccecb_block_size(const struct ccmode_ecb *mode)
{
    return mode->block_size;
}

CC_INLINE int ccecb_init(const struct ccmode_ecb *mode, ccecb_ctx *ctx, size_t key_len, const void *key)
{
    return mode->init(mode, ctx, key_len, key);
}

CC_INLINE int ccecb_update(const struct ccmode_ecb *mode, const ccecb_ctx *ctx, size_t nblocks, const void *in, void *out)
{
    return mode->ecb(ctx, nblocks, in, out);
}

CC_INLINE int
ccecb_one_shot(const struct ccmode_ecb *mode, size_t key_len, const void *key, size_t nblocks, const void *in, void *out)
{
    int rc;
    ccecb_ctx_decl(mode->size, ctx);
    rc = mode->init(mode, ctx, key_len, key);
    if (rc == 0) {
        rc = mode->ecb(ctx, nblocks, in, out);
    }
    ccecb_ctx_clear(mode->size, ctx);
    return rc;
}

/* CBC mode. */

/* The CBC interface changed due to rdar://11468135. This macros is to indicate
   to client which CBC API is implemented. Clients can support old versions of
   corecrypto at build time using this.
 */
#define __CC_HAS_FIX_FOR_11468135__ 1

/* Declare a cbc key named _name_.  Pass the size field of a struct ccmode_cbc
   for _size_. */
#define cccbc_ctx_decl(_size_, _name_) cc_ctx_decl(cccbc_ctx, _size_, _name_)
#define cccbc_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

/* Declare a cbc iv tweak named _name_.  Pass the blocksize field of a
   struct ccmode_cbc for _size_. */
#define cccbc_iv_decl(_size_, _name_) cc_ctx_decl(cccbc_iv, _size_, _name_)
#define cccbc_iv_clear(_size_, _name_) cc_clear(_size_, _name_)

/* Actual symmetric algorithm implementation can provide you one of these.

   Alternatively you can create a ccmode_cbc instance from any ccmode_ecb
   cipher.  To do so, statically initialize a struct ccmode_cbc using the
   CCMODE_FACTORY_CBC_DECRYPT or CCMODE_FACTORY_CBC_ENCRYPT macros.
   Alternatively you can dynamically initialize a struct ccmode_cbc
   ccmode_factory_cbc_decrypt() or ccmode_factory_cbc_encrypt(). */

CC_INLINE size_t cccbc_context_size(const struct ccmode_cbc *mode)
{
    return mode->size;
}

CC_INLINE size_t cccbc_block_size(const struct ccmode_cbc *mode)
{
    return mode->block_size;
}

CC_INLINE int cccbc_init(const struct ccmode_cbc *mode, cccbc_ctx *ctx, size_t key_len, const void *key)
{
    return mode->init(mode, ctx, key_len, key);
}

CC_INLINE int cccbc_set_iv(const struct ccmode_cbc *mode, cccbc_iv *iv_ctx, const void *iv)
{
    if (iv) {
        cc_copy(mode->block_size, iv_ctx, iv);
    } else {
        cc_clear(mode->block_size, iv_ctx);
    }
    return 0;
}

CC_INLINE int cccbc_update(const struct ccmode_cbc *mode, cccbc_ctx *ctx, cccbc_iv *iv, size_t nblocks, const void *in, void *out)
{
    return mode->cbc(ctx, iv, nblocks, in, out);
}

int cccbc_one_shot(const struct ccmode_cbc *mode,
                   size_t key_len,
                   const void *key,
                   const void *iv,
                   size_t nblocks,
                   const void *in,
                   void *out);

/* CFB mode. */

/* Declare a cfb key named _name_.  Pass the size field of a struct ccmode_cfb
   for _size_. */
#define cccfb_ctx_decl(_size_, _name_) cc_ctx_decl(cccfb_ctx, _size_, _name_)
#define cccfb_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

CC_INLINE size_t cccfb_context_size(const struct ccmode_cfb *mode)
{
    return mode->size;
}

CC_INLINE size_t cccfb_block_size(const struct ccmode_cfb *mode)
{
    return mode->block_size;
}

CC_INLINE int cccfb_init(const struct ccmode_cfb *mode, cccfb_ctx *ctx, size_t key_len, const void *key, const void *iv)
{
    return mode->init(mode, ctx, key_len, key, iv);
}

CC_INLINE int cccfb_update(const struct ccmode_cfb *mode, cccfb_ctx *ctx, size_t nbytes, const void *in, void *out)
{
    return mode->cfb(ctx, nbytes, in, out);
}

CC_INLINE int cccfb_one_shot(const struct ccmode_cfb *mode,
                             size_t key_len,
                             const void *key,
                             const void *iv,
                             size_t nbytes,
                             const void *in,
                             void *out)
{
    int rc;
    cccfb_ctx_decl(mode->size, ctx);
    rc = mode->init(mode, ctx, key_len, key, iv);
    if (rc == 0) {
        rc = mode->cfb(ctx, nbytes, in, out);
    }
    cccfb_ctx_clear(mode->size, ctx);
    return rc;
}

/* CFB8 mode. */

/* Declare a cfb8 key named _name_.  Pass the size field of a struct ccmode_cfb8
 for _size_. */
#define cccfb8_ctx_decl(_size_, _name_) cc_ctx_decl(cccfb8_ctx, _size_, _name_)
#define cccfb8_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

CC_INLINE size_t cccfb8_context_size(const struct ccmode_cfb8 *mode)
{
    return mode->size;
}

CC_INLINE size_t cccfb8_block_size(const struct ccmode_cfb8 *mode)
{
    return mode->block_size;
}

CC_INLINE int cccfb8_init(const struct ccmode_cfb8 *mode, cccfb8_ctx *ctx, size_t key_len, const void *key, const void *iv)
{
    return mode->init(mode, ctx, key_len, key, iv);
}

CC_INLINE int cccfb8_update(const struct ccmode_cfb8 *mode, cccfb8_ctx *ctx, size_t nbytes, const void *in, void *out)
{
    return mode->cfb8(ctx, nbytes, in, out);
}

CC_INLINE int cccfb8_one_shot(const struct ccmode_cfb8 *mode,
                              size_t key_len,
                              const void *key,
                              const void *iv,
                              size_t nbytes,
                              const void *in,
                              void *out)
{
    int rc;
    cccfb8_ctx_decl(mode->size, ctx);
    rc = mode->init(mode, ctx, key_len, key, iv);
    if (rc == 0) {
        rc = mode->cfb8(ctx, nbytes, in, out);
    }
    cccfb8_ctx_clear(mode->size, ctx);
    return rc;
}

/* CTR mode. */

/* Declare a ctr key named _name_.  Pass the size field of a struct ccmode_ctr
 for _size_. */
#define ccctr_ctx_decl(_size_, _name_) cc_ctx_decl(ccctr_ctx, _size_, _name_)
#define ccctr_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

/* This is Integer Counter Mode: The IV is the initial value of the counter
 that is incremented by 1 for each new block. Use the mode flags to select
 if the IV/Counter is stored in big or little endian. */

CC_INLINE size_t ccctr_context_size(const struct ccmode_ctr *mode)
{
    return mode->size;
}

CC_INLINE size_t ccctr_block_size(const struct ccmode_ctr *mode)
{
    return mode->block_size;
}

CC_INLINE int ccctr_init(const struct ccmode_ctr *mode, ccctr_ctx *ctx, size_t key_len, const void *key, const void *iv)
{
    return mode->init(mode, ctx, key_len, key, iv);
}

CC_INLINE int ccctr_update(const struct ccmode_ctr *mode, ccctr_ctx *ctx, size_t nbytes, const void *in, void *out)
{
    return mode->ctr(ctx, nbytes, in, out);
}

CC_INLINE int ccctr_one_shot(const struct ccmode_ctr *mode,
                             size_t key_len,
                             const void *key,
                             const void *iv,
                             size_t nbytes,
                             const void *in,
                             void *out)
{
    int rc;
    ccctr_ctx_decl(mode->size, ctx);
    rc = mode->init(mode, ctx, key_len, key, iv);
    if (rc == 0) {
        rc = mode->ctr(ctx, nbytes, in, out);
    }
    ccctr_ctx_clear(mode->size, ctx);
    return rc;
}

/* OFB mode. */

/* Declare a ofb key named _name_.  Pass the size field of a struct ccmode_ofb
 for _size_. */
#define ccofb_ctx_decl(_size_, _name_) cc_ctx_decl(ccofb_ctx, _size_, _name_)
#define ccofb_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

CC_INLINE size_t ccofb_context_size(const struct ccmode_ofb *mode)
{
    return mode->size;
}

CC_INLINE size_t ccofb_block_size(const struct ccmode_ofb *mode)
{
    return mode->block_size;
}

CC_INLINE int ccofb_init(const struct ccmode_ofb *mode, ccofb_ctx *ctx, size_t key_len, const void *key, const void *iv)
{
    return mode->init(mode, ctx, key_len, key, iv);
}

CC_INLINE int ccofb_update(const struct ccmode_ofb *mode, ccofb_ctx *ctx, size_t nbytes, const void *in, void *out)
{
    return mode->ofb(ctx, nbytes, in, out);
}

CC_INLINE int ccofb_one_shot(const struct ccmode_ofb *mode,
                             size_t key_len,
                             const void *key,
                             const void *iv,
                             size_t nbytes,
                             const void *in,
                             void *out)
{
    int rc;
    ccofb_ctx_decl(mode->size, ctx);
    rc = mode->init(mode, ctx, key_len, key, iv);
    if (rc == 0) {
        rc = mode->ofb(ctx, nbytes, in, out);
    }
    ccofb_ctx_clear(mode->size, ctx);
    return rc;
}

/* XTS mode. */

/* Declare a xts key named _name_.  Pass the size field of a struct ccmode_xts
 for _size_. */
#define ccxts_ctx_decl(_size_, _name_) cc_ctx_decl(ccxts_ctx, _size_, _name_)
#define ccxts_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

/* Declare a xts tweak named _name_.  Pass the tweak_size field of a
   struct ccmode_xts for _size_. */
#define ccxts_tweak_decl(_size_, _name_) cc_ctx_decl(ccxts_tweak, _size_, _name_)
#define ccxts_tweak_clear(_size_, _name_) cc_clear(_size_, _name_)

/* Actual symmetric algorithm implementation can provide you one of these.

 Alternatively you can create a ccmode_xts instance from any ccmode_ecb
 cipher.  To do so, statically initialize a struct ccmode_xts using the
 CCMODE_FACTORY_XTS_DECRYPT or CCMODE_FACTORY_XTS_ENCRYPT macros. Alternatively
 you can dynamically initialize a struct ccmode_xts
 ccmode_factory_xts_decrypt() or ccmode_factory_xts_encrypt(). */

/* NOTE that xts mode does not do cts padding.  It's really an xex mode.
   If you need cts padding use the ccpad_xts_encrypt and ccpad_xts_decrypt
   functions.   Also note that xts only works for ecb modes with a block_size
   of 16.  */

CC_INLINE size_t ccxts_context_size(const struct ccmode_xts *mode)
{
    return mode->size;
}

CC_INLINE size_t ccxts_block_size(const struct ccmode_xts *mode)
{
    return mode->block_size;
}

/*!
 @function   ccxts_init
 @abstract   Initialize an XTS context.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      key_nbytes Length of the key arguments in bytes
 @param      data_key   Key for data encryption
 @param      tweak_key  Key for tweak generation

 @result     0 iff successful.

 @discussion For security reasons, the two keys must be different.
 */
CC_INLINE int
ccxts_init(const struct ccmode_xts *mode, ccxts_ctx *ctx, size_t key_nbytes, const void *data_key, const void *tweak_key)
{
    return mode->init(mode, ctx, key_nbytes, data_key, tweak_key);
}

/*!
 @function   ccxts_set_tweak
 @abstract   Initialize the tweak for a sector.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      tweak      Context for the tweak for this sector
 @param      iv         Data used to generate the tweak

 @discussion The IV must be exactly one block in length.
 */
CC_INLINE int ccxts_set_tweak(const struct ccmode_xts *mode, ccxts_ctx *ctx, ccxts_tweak *tweak, const void *iv)
{
    return mode->set_tweak(ctx, tweak, iv);
}

/*!
 @function   ccxts_update
 @abstract   Encrypt or decrypt data.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for an instance
 @param      tweak      Context for the tweak for this sector
 @param      nblocks    Length of the data in blocks
 @param      in         Input data
 @param      out        Output buffer

 @result     The updated internal buffer of the tweak context. May be ignored.
  */
CC_INLINE void *
ccxts_update(const struct ccmode_xts *mode, ccxts_ctx *ctx, ccxts_tweak *tweak, size_t nblocks, const void *in, void *out)
{
    return mode->xts(ctx, tweak, nblocks, in, out);
}

/*!
 @function   ccxts_one_shot
 @abstract   Encrypt or decrypt data in XTS mode.

 @param      mode       Descriptor for the mode
 @param      key_nbytes Length of the key arguments in bytes
 @param      data_key   Key for data encryption
 @param      tweak_key  Key for tweak generation
 @param      iv         Data used to generate the tweak
 @param      nblocks    Length of the data in blocks
 @param      in         Input data
 @param      out        Output buffer

 @result     0 iff successful.

 @discussion For security reasons, the two keys must be different.
 */
int ccxts_one_shot(const struct ccmode_xts *mode,
                   size_t key_nbytes,
                   const void *data_key,
                   const void *tweak_key,
                   const void *iv,
                   size_t nblocks,
                   const void *in,
                   void *out);

/* Authenticated cipher modes. */

/* GCM mode. */

/* Declare a gcm key named _name_.  Pass the size field of a struct ccmode_gcm
 for _size_. */
#define ccgcm_ctx_decl(_size_, _name_) cc_ctx_decl(ccgcm_ctx, _size_, _name_)
#define ccgcm_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

#define CCGCM_IV_NBYTES 12
#define CCGCM_BLOCK_NBYTES 16

/* (2^32 - 2) blocks */
/* (2^36 - 32) bytes */
/* (2^39 - 256) bits */
/* Exceeding this figure breaks confidentiality and authenticity. */
#define CCGCM_TEXT_MAX_NBYTES ((1ULL << 36) - 32ULL)

CC_INLINE size_t ccgcm_context_size(const struct ccmode_gcm *mode)
{
    return mode->size;
}

CC_INLINE size_t ccgcm_block_size(const struct ccmode_gcm *mode)
{
    return mode->block_size;
}

/*!
 @function   ccgcm_init
 @abstract   Initialize a GCM context.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      key_nbytes Length of the key in bytes
 @param      key        Key for the underlying blockcipher (AES)

 @result     0 iff successful.

 @discussion The correct sequence of calls is:

 @code ccgcm_init(...)
 ccgcm_set_iv(...)
 ccgcm_aad(...)       (may be called zero or more times)
 ccgcm_update(...)    (may be called zero or more times)
 ccgcm_finalize(...)

 To reuse the context for additional encryptions, follow this sequence:

 @code ccgcm_reset(...)
 ccgcm_set_iv(...)
 ccgcm_aad(...)       (may be called zero or more times)
 ccgcm_update(...)    (may be called zero or more times)
 ccgcm_finalize(...)

 @warning The key-IV pair must be unique per encryption. The IV must be nonzero in length.

 @warning It is not permitted to call @p ccgcm_inc_iv after initializing the cipher via the @p ccgcm_init interface. Nonzero is
 returned in the event of an improper call sequence.

 @warning This function is not FIPS-compliant. Use @p ccgcm_init_with_iv instead.
 */
CC_INLINE int ccgcm_init(const struct ccmode_gcm *mode, ccgcm_ctx *ctx, size_t key_nbytes, const void *key)
{
    return mode->init(mode, ctx, key_nbytes, key);
}

/*!
 @function   ccgcm_init_with_iv
 @abstract   Initialize a GCM context to manage IVs internally.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      key_nbytes Length of the key in bytes
 @param      key        Key for the underlying blockcipher (AES)
 @param      iv         IV for the first encryption

 @result     0 iff successful.

 @discussion The correct sequence of calls is:

 @code ccgcm_init_with_iv(...)
 ccgcm_aad(...)       (may be called zero or more times)
 ccgcm_update(...)    (may be called zero or more times)
 ccgcm_finalize(...)

 To reuse the context for additional encryptions, follow this sequence:

 @code ccgcm_reset(...)
 ccgcm_inc_iv(...)
 ccgcm_aad(...)       (may be called zero or more times)
 ccgcm_update(...)    (may be called zero or more times)
 ccgcm_finalize(...)

 The IV must be exactly 12 bytes in length.

 Internally, the IV is treated as a four-byte salt followed by an eight-byte counter. This is to match the behavior of certain
 protocols (e.g. TLS). In the call to @p ccgcm_inc_iv, the counter component will be interpreted as a big-endian, unsigned value
 and incremented in place.

 @warning It is not permitted to call @p ccgcm_set_iv after initializing the cipher via the @p ccgcm_init_with_iv interface.
 Nonzero is returned in the event of an improper call sequence.

 @warning The security of GCM depends on the uniqueness of key-IV pairs. To avoid key-IV repetition, callers should not initialize
 multiple contexts with the same key material via the @p ccgcm_init_with_iv interface.
 */
int ccgcm_init_with_iv(const struct ccmode_gcm *mode, ccgcm_ctx *ctx, size_t key_nbytes, const void *key, const void *iv);

/*!
 @function   ccgcm_set_iv
 @abstract   Set the IV for encryption.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      iv_nbytes  Length of the IV in bytes
 @param      iv         Initialization vector

 @result     0 iff successful.

 @discussion Set the initialization vector for encryption.

 @warning The key-IV pair must be unique per encryption. The IV must be nonzero in length.

 In stateful protocols, if each packet exposes a guaranteed-unique value, it is recommended to format this as a 12-byte value for
 use as the IV.

 In stateless protocols, it is recommended to choose a 16-byte value using a cryptographically-secure pseudorandom number
 generator (e.g. @p ccrng).

 @warning This function may not be used after initializing the cipher via @p ccgcm_init_with_iv. Nonzero is returned in the event
 of an improper call sequence.

 @warning This function is not FIPS-compliant. Use @p ccgcm_init_with_iv instead.
 */
CC_INLINE int ccgcm_set_iv(const struct ccmode_gcm *mode, ccgcm_ctx *ctx, size_t iv_nbytes, const void *iv)
{
    return mode->set_iv(ctx, iv_nbytes, iv);
}

/*!
 @function   ccgcm_set_iv_legacy
 @abstract   Set the IV for encryption.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      iv_nbytes  Length of the IV in bytes
 @param      iv         Initialization vector

 @result     0 iff successful.

 @discussion Identical to @p ccgcm_set_iv except that it allows zero-length IVs.

 @warning Zero-length IVs nullify the authenticity guarantees of GCM.

 @warning Do not use this function in new applications.
 */
int ccgcm_set_iv_legacy(const struct ccmode_gcm *mode, ccgcm_ctx *ctx, size_t iv_nbytes, const void *iv);

/*!
 @function   ccgcm_inc_iv
 @abstract   Increment the IV for another encryption.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      iv         Updated initialization vector

 @result     0 iff successful.

 @discussion Updates the IV internally for another encryption.

 Internally, the IV is treated as a four-byte salt followed by an eight-byte counter. This is to match the behavior of certain
 protocols (e.g. TLS). The counter component is interpreted as a big-endian, unsigned value and incremented in place.

 The updated IV is copied to @p iv. This is to support protocols that require part of the IV to be specified explicitly in each
 packet (e.g. TLS).

 @warning This function may be used only after initializing the cipher via @p ccgcm_init_with_iv.
 */
int ccgcm_inc_iv(const struct ccmode_gcm *mode, ccgcm_ctx *ctx, void *iv);

/*!
 @function   ccgcm_aad
 @abstract   Authenticate additional data.

 @param      mode               Descriptor for the mode
 @param      ctx                Context for this instance
 @param      nbytes             Length of the additional data in bytes
 @param      additional_data    Additional data to authenticate

 @result     0 iff successful.

 @discussion This is typically used to authenticate data that cannot be encrypted (e.g. packet headers).

 This function may be called zero or more times.
 */
CC_INLINE int ccgcm_aad(const struct ccmode_gcm *mode, ccgcm_ctx *ctx, size_t nbytes, const void *additional_data)
{
    return mode->gmac(ctx, nbytes, additional_data);
}

/*!
 @function   ccgcm_gmac

 @discussion ccgcm_gmac is deprecated. Use the drop-in replacement 'ccgcm_aad' instead.
 */
CC_INLINE int ccgcm_gmac (const struct ccmode_gcm *mode, ccgcm_ctx *ctx, size_t nbytes, const void *in)
cc_deprecate_with_replacement("ccgcm_aad", 13.0, 10.15, 13.0, 6.0, 4.0)
{
    return mode->gmac(ctx, nbytes, in);
}

/*!
 @function   ccgcm_update
 @abstract   Encrypt or decrypt data.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      nbytes     Length of the data in bytes
 @param      in         Input plaintext or ciphertext
 @param      out        Output ciphertext or plaintext

 @result     0 iff successful.

 @discussion In-place processing is supported.

 This function may be called zero or more times.
 */
CC_INLINE int ccgcm_update(const struct ccmode_gcm *mode, ccgcm_ctx *ctx, size_t nbytes, const void *in, void *out)
{
    return mode->gcm(ctx, nbytes, in, out);
}

/*!
 @function   ccgcm_finalize
 @abstract   Finish processing and authenticate.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      tag_nbytes Length of the tag in bytes
 @param      tag        Authentication tag

 @result     0 iff successful.

 @discussion Finish processing a packet and generate the authentication tag.

 On encryption, @p tag is purely an output parameter. The generated tag is written to @p tag.

 On decryption, @p tag is both an input and an output parameter. Well-behaved callers should provide the authentication tag
 generated during encryption. The function will return nonzero if the input tag does not match the generated tag. The generated
 tag will be written into the @p tag buffer whether authentication succeeds or fails.

 @warning The generated tag is written to @p tag to support legacy applications that perform authentication manually. Do not
 follow this usage pattern in new applications. Rely on the function's error code to verify authenticity.
 */
CC_INLINE int ccgcm_finalize(const struct ccmode_gcm *mode, ccgcm_ctx *ctx, size_t tag_nbytes, void *tag)
{
    return mode->finalize(ctx, tag_nbytes, tag);
}

/*!
 @function   ccgcm_reset
 @abstract   Reset the context for another encryption.

 @param      mode       Descriptor for the mode
 @param      ctx        Context for this instance

 @result     0 iff successful.

 @discussion Refer to @p ccgcm_init for correct usage.
 */
CC_INLINE int ccgcm_reset(const struct ccmode_gcm *mode, ccgcm_ctx *ctx)
{
    return mode->reset(ctx);
}

/*!
 @function   ccgcm_one_shot
 @abstract   Encrypt or decrypt with GCM.

 @param      mode           Descriptor for the mode
 @param      key_nbytes     Length of the key in bytes
 @param      key            Key for the underlying blockcipher (AES)
 @param      iv_nbytes      Length of the IV in bytes
 @param      iv             Initialization vector
 @param      adata_nbytes   Length of the additional data in bytes
 @param      adata          Additional data to authenticate
 @param      nbytes         Length of the data in bytes
 @param      in             Input plaintext or ciphertext
 @param      out            Output ciphertext or plaintext
 @param      tag_nbytes     Length of the tag in bytes
 @param      tag            Authentication tag

 @result     0 iff successful.

 @discussion Perform GCM encryption or decryption.

 @warning The key-IV pair must be unique per encryption. The IV must be nonzero in length.

 In stateful protocols, if each packet exposes a guaranteed-unique value, it is recommended to format this as a 12-byte value for
 use as the IV.

 In stateless protocols, it is recommended to choose a 16-byte value using a cryptographically-secure pseudorandom number
 generator (e.g. @p ccrng).

 In-place processing is supported.

 On encryption, @p tag is purely an output parameter. The generated tag is written to @p tag.

 On decryption, @p tag is primarily an input parameter. The caller should provide the authentication tag generated during
 encryption. The function will return nonzero if the input tag does not match the generated tag.

 @warning To support legacy applications, @p tag is also an output parameter during decryption. The generated tag is written to @p
 tag. Legacy callers may choose to compare this to the tag generated during encryption. Do not follow this usage pattern in new
 applications.
 */
int ccgcm_one_shot(const struct ccmode_gcm *mode,
                   size_t key_nbytes,
                   const void *key,
                   size_t iv_nbytes,
                   const void *iv,
                   size_t adata_nbytes,
                   const void *adata,
                   size_t nbytes,
                   const void *in,
                   void *out,
                   size_t tag_nbytes,
                   void *tag);

/*!
 @function   ccgcm_one_shot_legacy
 @abstract   Encrypt or decrypt with GCM.

 @param      mode           Descriptor for the mode
 @param      key_nbytes     Length of the key in bytes
 @param      key            Key for the underlying blockcipher (AES)
 @param      iv_nbytes      Length of the IV in bytes
 @param      iv             Initialization vector
 @param      adata_nbytes   Length of the additional data in bytes
 @param      adata          Additional data to authenticate
 @param      nbytes         Length of the data in bytes
 @param      in             Input plaintext or ciphertext
 @param      out            Output ciphertext or plaintext
 @param      tag_nbytes     Length of the tag in bytes
 @param      tag            Authentication tag

 @result     0 iff successful.

 @discussion Identical to @p ccgcm_one_shot except that it allows zero-length IVs.

 @warning Zero-length IVs nullify the authenticity guarantees of GCM.

 @warning Do not use this function in new applications.
 */
int ccgcm_one_shot_legacy(const struct ccmode_gcm *mode,
                          size_t key_nbytes,
                          const void *key,
                          size_t iv_nbytes,
                          const void *iv,
                          size_t adata_nbytes,
                          const void *adata,
                          size_t nbytes,
                          const void *in,
                          void *out,
                          size_t tag_nbytes,
                          void *tag);

/* CCM */

#define ccccm_ctx_decl(_size_, _name_) cc_ctx_decl(ccccm_ctx, _size_, _name_)
#define ccccm_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

/* Declare a ccm nonce named _name_.  Pass the mode->nonce_ctx_size for _size_. */
#define ccccm_nonce_decl(_size_, _name_) cc_ctx_decl(ccccm_nonce, _size_, _name_)
#define ccccm_nonce_clear(_size_, _name_) cc_clear(_size_, _name_)

CC_INLINE size_t ccccm_context_size(const struct ccmode_ccm *mode)
{
    return mode->size;
}

CC_INLINE size_t ccccm_block_size(const struct ccmode_ccm *mode)
{
    return mode->block_size;
}

CC_INLINE int ccccm_init(const struct ccmode_ccm *mode, ccccm_ctx *ctx, size_t key_len, const void *key)
{
    return mode->init(mode, ctx, key_len, key);
}

CC_INLINE int ccccm_set_iv(const struct ccmode_ccm *mode,
                           ccccm_ctx *ctx,
                           ccccm_nonce *nonce_ctx,
                           size_t nonce_len,
                           const void *nonce,
                           size_t mac_size,
                           size_t auth_len,
                           size_t data_len)
{
    return mode->set_iv(ctx, nonce_ctx, nonce_len, nonce, mac_size, auth_len, data_len);
}

CC_INLINE int ccccm_cbcmac(const struct ccmode_ccm *mode, ccccm_ctx *ctx, ccccm_nonce *nonce_ctx, size_t nbytes, const void *in)
{
    return mode->cbcmac(ctx, nonce_ctx, nbytes, in);
}

CC_INLINE int
ccccm_update(const struct ccmode_ccm *mode, ccccm_ctx *ctx, ccccm_nonce *nonce_ctx, size_t nbytes, const void *in, void *out)
{
    return mode->ccm(ctx, nonce_ctx, nbytes, in, out);
}

CC_INLINE int ccccm_finalize(const struct ccmode_ccm *mode, ccccm_ctx *ctx, ccccm_nonce *nonce_ctx, void *mac)
{
    return mode->finalize(ctx, nonce_ctx, mac);
}

CC_INLINE int ccccm_reset(const struct ccmode_ccm *mode, ccccm_ctx *ctx, ccccm_nonce *nonce_ctx)
{
    return mode->reset(ctx, nonce_ctx);
}

CC_INLINE int ccccm_one_shot(const struct ccmode_ccm *mode,
                             size_t key_len,
                             const void *key,
                             size_t nonce_len,
                             const void *nonce,
                             size_t nbytes,
                             const void *in,
                             void *out,
                             size_t adata_len,
                             const void *adata,
                             size_t mac_size,
                             void *mac)
{
    int rc;
    ccccm_ctx_decl(mode->size, ctx);
    ccccm_nonce_decl(mode->nonce_size, nonce_ctx);
    rc = mode->init(mode, ctx, key_len, key);
    if (rc == 0) {
        rc = mode->set_iv(ctx, nonce_ctx, nonce_len, nonce, mac_size, adata_len, nbytes);
    }
    if (rc == 0) {
        rc = mode->cbcmac(ctx, nonce_ctx, adata_len, adata);
    }
    if (rc == 0) {
        rc = mode->ccm(ctx, nonce_ctx, nbytes, in, out);
    }
    if (rc == 0) {
        rc = mode->finalize(ctx, nonce_ctx, mac);
    }
    ccccm_ctx_clear(mode->size, ctx);
    ccccm_nonce_clear(mode->nonce_size, nonce_ctx);

    return rc;
}

/* OMAC mode. */

/* Declare a omac key named _name_.  Pass the size field of a struct ccmode_omac
 for _size_. */
#define ccomac_ctx_decl(_size_, _name_) cc_ctx_decl(ccomac_ctx, _size_, _name_)
#define ccomac_ctx_clear(_size_, _name_) cc_clear(_size_, _name_)

CC_INLINE size_t ccomac_context_size(const struct ccmode_omac *mode)
{
    return mode->size;
}

CC_INLINE size_t ccomac_block_size(const struct ccmode_omac *mode)
{
    return mode->block_size;
}

CC_INLINE int ccomac_init(const struct ccmode_omac *mode, ccomac_ctx *ctx, size_t tweak_len, size_t key_len, const void *key)
{
    return mode->init(mode, ctx, tweak_len, key_len, key);
}

CC_INLINE int
ccomac_update(const struct ccmode_omac *mode, ccomac_ctx *ctx, size_t nblocks, const void *tweak, const void *in, void *out)
{
    return mode->omac(ctx, nblocks, tweak, in, out);
}

CC_INLINE int ccomac_one_shot(const struct ccmode_omac *mode,
                              size_t tweak_len,
                              size_t key_len,
                              const void *key,
                              const void *tweak,
                              size_t nblocks,
                              const void *in,
                              void *out)
{
    int rc;
    ccomac_ctx_decl(mode->size, ctx);
    rc = mode->init(mode, ctx, tweak_len, key_len, key);
    if (rc == 0) {
        rc = mode->omac(ctx, nblocks, tweak, in, out);
    }
    ccomac_ctx_clear(mode->size, ctx);
    return rc;
}

#endif /* _CORECRYPTO_CCMODE_H_ */
