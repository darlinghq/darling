/*
 *  ccpad.h
 *  corecrypto
 *
 *  Created on 12/07/2010
 *
 *  Copyright (c) 2010,2011,2012,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCPAD_H_
#define _CORECRYPTO_CCPAD_H_

#include <corecrypto/ccmode.h>

// CTS1,2,3 are defined in Addendum to 800-38A,
// "Cipher Modes of Operation: Three Variants of Ciphertext Stealing for CBC Mode"
// CTS3 is also known as "CTS" in RFC3962

/* Contract is nbytes is at least 1 block + 1 byte.  Also in is nbytes long out is nbytes long. */
size_t ccpad_cts1_decrypt(const struct ccmode_cbc *cbc, cccbc_ctx *ctx, cccbc_iv *iv,
                       size_t nbytes, const void *in, void *out);

/* Contract is nbytes is at least 1 block + 1 byte.  Also in is nbytes long out is nbytes long. */
size_t ccpad_cts1_encrypt(const struct ccmode_cbc *cbc, cccbc_ctx *ctx, cccbc_iv *iv,
                       size_t nbytes, const void *in, void *out);
/* Contract is nbytes is at least 1 block + 1 byte.  Also in is nbytes long out is nbytes long. */
size_t ccpad_cts2_decrypt(const struct ccmode_cbc *cbc, cccbc_ctx *ctx, cccbc_iv *iv,
                       size_t nbytes, const void *in, void *out);

/* Contract is nbytes is at least 1 block + 1 byte.  Also in is nbytes long out is nbytes long. */
size_t ccpad_cts2_encrypt(const struct ccmode_cbc *cbc, cccbc_ctx *ctx, cccbc_iv *iv,
                       size_t nbytes, const void *in, void *out);
/* Contract is nbytes is at least 1 block + 1 byte.  Also in is nbytes long out is nbytes long. */
size_t ccpad_cts3_decrypt(const struct ccmode_cbc *cbc, cccbc_ctx *ctx, cccbc_iv *iv,
                       size_t nbytes, const void *in, void *out);

/* Contract is nbytes is at least 1 block + 1 byte.  Also in is nbytes long out is nbytes long. */
size_t ccpad_cts3_encrypt(const struct ccmode_cbc *cbc, cccbc_ctx *ctx, cccbc_iv *iv,
                       size_t nbytes, const void *in, void *out);

/* Contract is nbytes is non zero and a multiple of block_size. Furthermore in is nbytes long and out is nbytes long.  Returns number of bytes written to out (technically we always write nbytes to out but the returned value is the number of bytes decrypted after removal of padding.

    To be safe we remove the entire offending block if the pkcs7 padding checks failed.  However we purposely don't report the failure to decode the padding since any use of this error leads to potential security exploits.  So currently there is no way to distinguish between a full block of padding and bad padding.
 */
size_t ccpad_pkcs7_decrypt(const struct ccmode_cbc *cbc, cccbc_ctx *ctx, cccbc_iv *iv,
                           size_t nbytes, const void *in, void *out);

/* Contract is in is nbytes long.  Writes (nbytes / block_size) + 1 times block_size to out.  In other words, out must be nbytes rounded down to the closest multiple of block_size plus block_size bytes. */
size_t ccpad_pkcs7_encrypt(const struct ccmode_cbc *cbc, cccbc_ctx *ctx, cccbc_iv *iv,
                         size_t nbytes, const void *in, void *out);

/* Contract is 'don't break CommonCrypto functionality that allows PKCS7 padding with ECB mode'.  This is basically the same routines above, without an IV, because calling
   crypt with an IV makes ecb cry (and crash) */

size_t ccpad_pkcs7_ecb_decrypt(const struct ccmode_ecb *ecb, ccecb_ctx *ecb_key,
                               size_t nbytes, const void *in, void *out);

size_t ccpad_pkcs7_ecb_encrypt(const struct ccmode_ecb *ecb, ccecb_ctx *ctx,
                             size_t nbytes, const void *in, void *out);

/* Function common to ccpad_pkcs7_ecb_decrypt and ccpad_pkcs7_decrypt */
size_t ccpad_pkcs7_decode(const size_t block_size, const uint8_t* last_block);

/* Contract is nbytes is at least 1 block + 1 byte.  Also in is nbytes long out is nbytes long. */
size_t ccpad_xts_decrypt(const struct ccmode_xts *xts, ccxts_ctx *ctx, ccxts_tweak *tweak,
                       size_t nbytes, const void *in, void *out);

/* Contract is nbytes is at least 1 block + 1 byte.  Also in is nbytes long out is nbytes long. */
void ccpad_xts_encrypt(const struct ccmode_xts *xts, ccxts_ctx *ctx, ccxts_tweak *tweak,
                       size_t nbytes, const void *in, void *out);

#endif /* _CORECRYPTO_CCPAD_H_ */
