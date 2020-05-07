/*
	ccchacha20poly1305.h
	corecrypto

	Copyright 2014 Apple Inc. All rights reserved.
*/

#ifndef _CORECRYPTO_CCCHACHA20POLY1305_H_
#define _CORECRYPTO_CCCHACHA20POLY1305_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CCCHACHA20_KEY_NBYTES 32
#define CCCHACHA20_BLOCK_NBYTES 64
#define CCCHACHA20_BLOCK_NBITS (CCCHACHA20_BLOCK_NBYTES * 8)
#define CCCHACHA20_NONCE_NBYTES 12

typedef struct {
	uint32_t state[16];
	uint8_t	buffer[CCCHACHA20_BLOCK_NBYTES];
	size_t leftover;
} ccchacha20_ctx;

#define CCPOLY1305_TAG_NBYTES 16

typedef struct {
	uint32_t r0, r1, r2, r3, r4;
	uint32_t s1, s2, s3, s4;
	uint32_t h0, h1, h2, h3, h4;
	uint8_t	buf[16];
	size_t buf_used;
	uint8_t	key[16];
} ccpoly1305_ctx;


/*!
 @group		ccchacha20poly1305
 @abstract	Encrypts and authenticates or decrypts and verifies data.
 @discussion	See RFC 7539 for details.

 @warning The key-nonce pair must be unique per encryption.

 @warning A single message can be at most (2^38 - 64) bytes in length.

 The correct sequence of calls to encrypt is:

 @code ccchacha20poly1305_init(...)
 ccchacha20poly1305_setnonce(...)
 ccchacha20poly1305_aad(...)       (may be called zero or more times)
 ccchacha20poly1305_encrypt(...)   (may be called zero or more times)
 ccchacha20poly1305_finalize(...)

 To reuse the context for additional encryptions, follow this sequence:

 @code ccchacha20poly1305_reset(...)
 ccchacha20poly1305_setnonce(...)
 ccchacha20poly1305_aad(...)       (may be called zero or more times)
 ccchacha20poly1305_encrypt(...)   (may be called zero or more times)
 ccchacha20poly1305_finalize(...)

 To decrypt, follow this call sequence:

 @code ccchacha20poly1305_init(...)
 ccchacha20poly1305_setnonce(...)
 ccchacha20poly1305_aad(...)       (may be called zero or more times)
 ccchacha20poly1305_decrypt(...)   (may be called zero or more times)
 ccchacha20poly1305_verify(...)    (returns zero on successful decryption)

 To reuse the context for additional encryptions, follow this sequence:

 @code ccchacha20poly1305_reset(...)
 ccchacha20poly1305_setnonce(...)
 ccchacha20poly1305_aad(...)       (may be called zero or more times)
 ccchacha20poly1305_decrypt(...)   (may be called zero or more times)
 ccchacha20poly1305_verify(...)    (returns zero on successful decryption)
*/

#define CCCHACHA20POLY1305_KEY_NBYTES (CCCHACHA20_KEY_NBYTES)
#define CCCHACHA20POLY1305_NONCE_NBYTES (CCCHACHA20_NONCE_NBYTES)
#define CCCHACHA20POLY1305_TAG_NBYTES (CCPOLY1305_TAG_NBYTES)

/* (2^32 - 1) blocks */
/* (2^38 - 64) bytes */
/* (2^41 - 512) bits */
/* Exceeding this figure breaks confidentiality and authenticity. */
#define CCCHACHA20POLY1305_TEXT_MAX_NBYTES ((1ULL << 38) - 64ULL)

#define CCCHACHA20POLY1305_STATE_SETNONCE 1
#define CCCHACHA20POLY1305_STATE_AAD 2
#define CCCHACHA20POLY1305_STATE_ENCRYPT 3
#define CCCHACHA20POLY1305_STATE_DECRYPT 4
#define CCCHACHA20POLY1305_STATE_FINAL 5

typedef struct {
	ccchacha20_ctx chacha20_ctx;
	ccpoly1305_ctx poly1305_ctx;
	uint64_t aad_nbytes;
	uint64_t text_nbytes;
    uint8_t state;
} ccchacha20poly1305_ctx;

// This is just a stub right now.
// Eventually we will optimize by platform.
struct ccchacha20poly1305_info {

};

const struct ccchacha20poly1305_info *ccchacha20poly1305_info(void);

/*!
 @function   ccchacha20poly1305_init
 @abstract   Initialize a chacha20poly1305 context.

 @param      info       Implementation descriptor
 @param      ctx        Context for this instance
 @param      key        Secret chacha20 key

 @result     0 iff successful.

 @discussion The key is 32 bytes in length.

 @warning The key-nonce pair must be unique per encryption.
 */
int	ccchacha20poly1305_init(const struct ccchacha20poly1305_info *info, ccchacha20poly1305_ctx *ctx, const uint8_t *key);

/*!
 @function   ccchacha20poly1305_reset
 @abstract   Reset a chacha20poly1305 context for reuse.

 @param      info       Implementation descriptor
 @param      ctx        Context for this instance

 @result     0 iff successful.
 */
int ccchacha20poly1305_reset(const struct ccchacha20poly1305_info *info, ccchacha20poly1305_ctx *ctx);

/*!
 @function   ccchacha20poly1305_setnonce
 @abstract   Set the nonce for encryption or decryption.

 @param      info       Implementation descriptor
 @param      ctx        Context for this instance
 @param      nonce      Unique nonce per encryption

 @result     0 iff successful.

 @discussion The nonce is 12 bytes in length.

 @warning The key-nonce pair must be unique per encryption.
 */
int ccchacha20poly1305_setnonce(const struct ccchacha20poly1305_info *info, ccchacha20poly1305_ctx *ctx, const uint8_t *nonce);
int ccchacha20poly1305_incnonce(const struct ccchacha20poly1305_info *info, ccchacha20poly1305_ctx *ctx, uint8_t *nonce);

/*!
 @function   ccchacha20poly1305_aad
 @abstract   Authenticate additional data.

 @param      info       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      nbytes     Length of the additional data in bytes
 @param      aad        Additional data to authenticate

 @result     0 iff successful.

 @discussion This is typically used to authenticate data that cannot be encrypted (e.g. packet headers).

 This function may be called zero or more times.
 */
int	ccchacha20poly1305_aad(const struct ccchacha20poly1305_info *info, ccchacha20poly1305_ctx *ctx, size_t nbytes, const void *aad);

/*!
 @function   ccchacha20poly1305_encrypt
 @abstract   Encrypt data.

 @param      info       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      nbytes     Length of the plaintext in bytes
 @param      ptext      Input plaintext
 @param      ctext      Output ciphertext

 @result     0 iff successful.

 @discussion In-place processing is supported.

 This function may be called zero or more times.
 */
int	ccchacha20poly1305_encrypt(const struct ccchacha20poly1305_info *info, ccchacha20poly1305_ctx *ctx, size_t nbytes, const void *ptext, void *ctext);

/*!
 @function   ccchacha20poly1305_finalize
 @abstract   Finalize encryption.

 @param      info       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      tag        Generated authentication tag

 @result     0 iff successful.

 @discussion The generated tag is 16 bytes in length.
 */
int	ccchacha20poly1305_finalize(const struct ccchacha20poly1305_info *info, ccchacha20poly1305_ctx *ctx, uint8_t *tag);

/*!
 @function   ccchacha20poly1305_decrypt
 @abstract   Decrypt data.

 @param      info       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      nbytes     Length of the ciphertext in bytes
 @param      ctext      Input ciphertext
 @param      ptext      Output plaintext

 @result     0 iff successful.

 @discussion In-place processing is supported.

 This function may be called zero or more times.
 */
int	ccchacha20poly1305_decrypt(const struct ccchacha20poly1305_info *info, ccchacha20poly1305_ctx *ctx, size_t nbytes, const void *ctext, void *ptext);

/*!
 @function   ccchacha20poly1305_verify
 @abstract   Verify authenticity.

 @param      info       Descriptor for the mode
 @param      ctx        Context for this instance
 @param      tag        Expected authentication tag

 @result     0 iff authentic and otherwise successful.

 @discussion The expected tag is 16 bytes in length.
 */
int	ccchacha20poly1305_verify(const struct ccchacha20poly1305_info *info, ccchacha20poly1305_ctx *ctx, const uint8_t *tag);

/*!
 @function      ccchacha20poly1305_encrypt_oneshot
 @abstract      Encrypt with chacha20poly1305.

 @param      info           Descriptor for the mode
 @param      key            Secret chacha20 key
 @param      nonce          Unique nonce per encryption
 @param      aad_nbytes     Length of the additional data in bytes
 @param      aad            Additional data to authenticate
 @param      ptext_nbytes   Length of the plaintext in bytes
 @param      ptext          Input plaintext
 @param      ctext          Output ciphertext
 @param      tag            Generated authentication tag

 @discussion See RFC 7539 for details.

 The key is 32 bytes in length.

 The nonce is 12 bytes in length.

 The generated tag is 16 bytes in length.

 In-place processing is supported.

 @warning The key-nonce pair must be unique per encryption.

 @warning A single message can be at most (2^38 - 64) bytes in length.
 */
int ccchacha20poly1305_encrypt_oneshot(const struct ccchacha20poly1305_info *info, const uint8_t *key, const uint8_t *nonce, size_t aad_nbytes, const void *aad, size_t ptext_nbytes, const void *ptext, void *ctext, uint8_t *tag);

/*!
 @function      ccchacha20poly1305_decrypt_oneshot
 @abstract      Decrypt with chacha20poly1305.

 @param      info           Descriptor for the mode
 @param      key            Secret chacha20 key
 @param      nonce          Unique nonce per encryption
 @param      aad_nbytes     Length of the additional data in bytes
 @param      aad            Additional data to authenticate
 @param      ctext_nbytes   Length of the ciphertext in bytes
 @param      ctext          Input ciphertext
 @param      ptext          Output plaintext
 @param      tag            Expected authentication tag

 @discussion See RFC 7539 for details.

 The key is 32 bytes in length.

 The nonce is 12 bytes in length.

 The generated tag is 16 bytes in length.

 In-place processing is supported.
 */
int ccchacha20poly1305_decrypt_oneshot(const struct ccchacha20poly1305_info *info, const uint8_t *key, const uint8_t *nonce, size_t aad_nbytes, const void *aad, size_t ctext_nbytes, const void *ctext, void *ptext, const uint8_t *tag);

#endif
