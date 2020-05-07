/*
 *  ccrsa.h
 *  corecrypto
 *
 *  Created on 11/16/2010
 *
 *  Copyright (c) 2010,2011,2012,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCRSA_H_
#define _CORECRYPTO_CCRSA_H_

#include <corecrypto/cc.h>
#include <corecrypto/ccdigest.h>
#include <corecrypto/ccrng.h>
#include <corecrypto/cczp.h>
#include <stdbool.h>

// Apple does not generate keys of greater than 4096 bits
// This limit is relaxed to accommodate potential third-party consumers
#define CCRSA_KEYGEN_MAX_NBITS 8192

struct ccrsa_full_ctx {
    __CCZP_ELEMENTS_DEFINITIONS(pb_)
} CC_ALIGNED(CCN_UNIT_SIZE);

struct ccrsa_pub_ctx {
    __CCZP_ELEMENTS_DEFINITIONS(pb_)
} CC_ALIGNED(CCN_UNIT_SIZE);

struct ccrsa_priv_ctx {
    __CCZP_ELEMENTS_DEFINITIONS(pv_)
} CC_ALIGNED(CCN_UNIT_SIZE);

typedef struct ccrsa_full_ctx* ccrsa_full_ctx_t;
typedef struct ccrsa_pub_ctx* ccrsa_pub_ctx_t;
typedef struct ccrsa_priv_ctx* ccrsa_priv_ctx_t;

/*
 public key cczp      d=e^-1 mod phi(m) priv key cczp             priv key cczq             dp, dq, qinv
 |                              |       |                         |                         |
 |                              |       |                         |                         |
 +-------+------+-------+------++------++-------+------+---------++-------+------+---------++-------+-------+---------+
 | zm_hd | m[n] |mr[n+1]| e[n] || d[n] || zp_hd |p[n/2]|pr[n/2+1]|| zq_hd |q[n/2]|qr[n/2+1]||dp[n/2]|dq[n/2]|qinv[n/2]|
 +-------+------+-------+------++------++-------+------+---------++-------+------+---------++-------+-------+---------+
 */

 /* Return the size of an ccec_full_ctx where each ccn is _size_ bytes. Get _size_ through ccn_sizeof(nbits) */

/* Return the size of an ccec_full_ctx where each ccn is _size_ bytes. */

#define ccrsa_pub_ctx_size(_size_)   (sizeof(struct cczp) + CCN_UNIT_SIZE + 3 * (_size_))
#define ccrsa_priv_ctx_size(_size_)  ((sizeof(struct cczp) + CCN_UNIT_SIZE) * 2 + 7 * ccn_sizeof(ccn_bitsof_size(_size_)/2 + 1))
#define ccrsa_full_ctx_size(_size_)  (ccrsa_pub_ctx_size(_size_) + _size_ + ccrsa_priv_ctx_size(_size_))

/* Declare a fully scheduled rsa key.  Size is the size in bytes each ccn in
   the key.  For example to declare (on the stack or in a struct) a 1021 bit
   rsa public key named foo use ccrsa_pub_ctx_decl(ccn_sizeof(1021), foo). 
 */
#define ccrsa_full_ctx_decl(_size_, _name_)   cc_ctx_decl(struct ccrsa_full_ctx, ccrsa_full_ctx_size(_size_), _name_)
#define ccrsa_full_ctx_clear(_size_, _name_)  cc_clear(ccrsa_full_ctx_size(_size_), _name_)
#define ccrsa_pub_ctx_decl(_size_, _name_)    cc_ctx_decl(struct ccrsa_pub_ctx, ccrsa_pub_ctx_size(_size_), _name_)
#define ccrsa_pub_ctx_clear(_size_, _name_)   cc_clear(ccrsa_pub_ctx_size(_size_), _name_)

// accessors to ccrsa full and public key fields. */
// The offsets are computed using pb_ccn. If any object other than ccrsa_full_ctx_t
// or ccrsa_pub_ctx_t is passed to the macros, compiler error is generated.

#define ccrsa_ctx_zm(_ctx_)        ((cczp_t)(_ctx_))
#define ccrsa_ctx_n(_ctx_)         (ccrsa_ctx_zm(_ctx_)->n)
#define ccrsa_ctx_m(_ctx_)         ((_ctx_)->pb_ccn)

#define ccrsa_ctx_e(_ctx_)         (ccrsa_ctx_m(_ctx_) + 2 * ccrsa_ctx_n(_ctx_) + 1)
#define ccrsa_ctx_d(_ctx_)         (ccrsa_ctx_m(_ctx_) + 3 * ccrsa_ctx_n(_ctx_) + 1)

// accessors to ccrsa private key fields
// The offsets are computed using pv_ccn. If any object other than ccrsa_priv_ctx_t
// is passed to the macros, compiler error is generated.
#define ccrsa_ctx_private_zp(FK)   ((cczp_t)ccrsa_get_private_ctx_ptr(FK))
#define ccrsa_ctx_private_zq(FK)   ((cczp_t)((ccrsa_get_private_ctx_ptr(FK))->pv_ccn + 2 * ccrsa_ctx_private_zp(FK)->n + 1))
#define ccrsa_ctx_private_dp(FK)   ((ccrsa_get_private_ctx_ptr(FK))->pv_ccn + 4 * ccrsa_ctx_private_zp(FK)->n + 2 + ccn_nof_size(sizeof(struct cczp)))
#define ccrsa_ctx_private_dq(FK)   ((ccrsa_get_private_ctx_ptr(FK))->pv_ccn + 5 * ccrsa_ctx_private_zp(FK)->n + 2 + ccn_nof_size(sizeof(struct cczp)))
#define ccrsa_ctx_private_qinv(FK) ((ccrsa_get_private_ctx_ptr(FK))->pv_ccn + 6 * ccrsa_ctx_private_zp(FK)->n + 2 + ccn_nof_size(sizeof(struct cczp)))

/* rvalue accessors to ccec_key fields. */
CC_INLINE
ccrsa_priv_ctx_t ccrsa_get_private_ctx_ptr(ccrsa_full_ctx_t fk) {
    ccrsa_priv_ctx_t priv = (ccrsa_priv_ctx_t)(ccrsa_ctx_d(fk)+ccrsa_ctx_n(fk));
    return priv;
}

/*!
 @function   ccrsa_ctx_public
 @abstract   gets the public key from full key
 @param      fk      RSA full key
 @result     Returns RSA public ker
 */
CC_INLINE
ccrsa_pub_ctx_t ccrsa_ctx_public(ccrsa_full_ctx_t fk) {
    return (ccrsa_pub_ctx_t) fk;
}

/* Return exact key bit size */
CC_NONNULL_ALL
size_t ccrsa_pubkeylength(ccrsa_pub_ctx_t pubk);

/* PKCS1 pad_markers */
#define CCRSA_PKCS1_PAD_SIGN     1
#define CCRSA_PKCS1_PAD_ENCRYPT  2

/* Initialize key based on modulus and e as cc_unit.  key->zp.n must already be set. */
CC_NONNULL((1, 2, 3))
int ccrsa_init_pub(ccrsa_pub_ctx_t key, const cc_unit *modulus,
                    const cc_unit *e);

/*!
 @function   ccrsa_make_priv
 @abstract   Initialize public and private key based on modulus and e, p and q as big endian byte arrays;
 
 @param      full_ctx   Initialized context with full_ctx->zp.n already set to 2*ccn_nof_size(p_mbytes)
 @param      exp_mbytes Number of bytes in big endian e.
 @param      exp_in     pointer to big endian exponent e (may have leading 0's).
 @param      p_mbytes   Number of bytes in big endian p.
 @param      p_in       Pointer to the rsa p.
 @param      q_mbytes   Number of bytes in big endian q.
 @param      q_in       Pointer to the rsa q.
 @result     0          iff successful.
 
 @discussion  full_ctx->zp.n must already be set to 2*ccn_nof_size(p_mbytes), witt the expectation that p_mbytes>q_mbytes.
 e is the public exponent, and exp_mbytes<= 2*p_mbytes.
 The output is a fully formed rsa context with N=pq, d=e^{-1} mod phi(N), and appropriate inverses of different associated values precomputed
 to speed computation.
 */

int ccrsa_make_priv(ccrsa_full_ctx_t full_ctx,
                    size_t exp_mbytes,
                    const uint8_t *exp_in,
                    size_t p_mbytes,
                    const uint8_t *p_in,
                    size_t q_mbytes,
                    const uint8_t *q_in);

/* Initialize key based on modulus and e as big endian byte array
    key->zp.n must already be set. */
CC_NONNULL((1, 3, 5))
int ccrsa_make_pub(ccrsa_pub_ctx_t pubk,
                              size_t exp_nbytes, const uint8_t *exp,
                              size_t mod_nbytes, const uint8_t *mod);

/* Do a public key crypto operation (typically verify or encrypt) on in and put
   the result in out. Both in and out should be cc_unit aligned and
   ccrsa_key_n(key) units long. Clients should use ccn_read_uint() to
   convert bytes to a cc_unit to use for this API.*/
CC_NONNULL((1, 2, 3))
int ccrsa_pub_crypt(ccrsa_pub_ctx_t key, cc_unit *out, const cc_unit *in);

/* Generate an nbit rsa key pair in key, which should be allocated using
   ccrsa_full_ctx_decl(ccn_sizeof(1024), rsa_ctx). The unsigned big endian
   byte array exponent e of length e_size is used as the exponent. It's an
   error to call this function with an exponent larger than nbits. rng
   must be a pointer to an initialized struct ccrng_state. */
CC_NONNULL((2, 4, 5))
int ccrsa_generate_key(size_t nbits, ccrsa_full_ctx_t rsa_ctx,
                       size_t e_size, const void *e, struct ccrng_state *rng) CC_WARN_RESULT;

/* Generate RSA key in conformance with FIPS186-4 standard.
   The first RNG `rng` will be used to generate p and q.
   The second RNG `rng_mr` will be used only for primality testing.
   This is relevant only for testing, just pass the same RNG twice. */
CC_NONNULL((2, 4, 5, 6))
int
ccrsa_generate_fips186_key(size_t nbits, ccrsa_full_ctx_t fk,
                           size_t e_size, const void *eBytes,
                           struct ccrng_state *rng, struct ccrng_state *rng_mr) CC_WARN_RESULT;

/* Construct RSA key from fix input in conformance with FIPS186-4 standard */
CC_NONNULL((3, 5, 7, 9, 11, 13, 15, 16))
int
ccrsa_make_fips186_key(size_t nbits,
                       const cc_size e_n, const cc_unit *e,
                       const cc_size xp1Len, const cc_unit *xp1, const cc_size xp2Len, const cc_unit *xp2,
                       const cc_size xpLen, const cc_unit *xp,
                       const cc_size xq1Len, const cc_unit *xq1, const cc_size xq2Len, const cc_unit *xq2,
                       const cc_size xqLen, const cc_unit *xq,
                       ccrsa_full_ctx_t fk,
                       cc_size *np, cc_unit *r_p,
                       cc_size *nq, cc_unit *r_q,
                       cc_size *nm, cc_unit *r_m,
                       cc_size *nd, cc_unit *r_d);

/*!
 * @brief ccrsa_sign_pss() generates RSASSA-PSS signature in PKCS1-V2 format
 *
 * note that in RSASSA-PSS, salt length is part of the signature as specified in ASN1
 * RSASSA-PSS-params ::= SEQUENCE {
 * hashAlgorithm      [0] HashAlgorithm      DEFAULT sha1,
 * maskGenAlgorithm   [1] MaskGenAlgorithm   DEFAULT mgf1SHA1,
 * saltLength         [2] INTEGER            DEFAULT 20,
 * trailerField       [3] TrailerField       DEFAULT trailerFieldBC
 *
 *
 * FIPS 186-4 for RSASSA-PSS:
 * .... Both signature schemes are approved for use, but additional constraints are imposed beyond those specified in PKCS #1 v2.1.....
 *
 * • If nlen = 1024 bits (i.e., 128 bytes), and the output length of the approved hash function output block is 512 bits (i.e., 64 bytes), then the length (in bytes) of the salt (sLen) shall satisfy 0 ≤ sLen ≤ hLen – 2,
 * • Otherwise, the length (in bytes) of the salt (sLen) shall satisfy 0 ≤ sLen ≤ hLen, where hLen is the length of the hash function output block (in bytes).
 *
 *
 * • CAVS test vectors are not very useful in the case of RSA-PSS, because they only validate the exponentiation part of the signature. See: http://csrc.nist.gov/groups/STM/cavp/documents/components/RSA2SP1VS.pdf
 *
 * @param	key              The RSA key
 * @param	hashAlgorithm    The hash algorithm used to generate mHash from the original message. It is also used inside the PSS encoding function. This is also the hash function to be used in the mask generation function (MGF)
 * @param   MgfHashAlgorithm The hash algorithm for thr mask generation function
 * @param   rng              Random number geberator to generate salt in PSS encoding
 * @param	saltSize          Intended length of the salt
 * @param   hSize             Length of message hash . Must be equal to hashAlgorithm->output_size
 * @param	mHash            The input that needs to be signed. This is the hash of message M with length of hLen
 *
 * @param   sig              The signature output
 * @param   sigSize           The length of generated signature in bytes, which equals the size of the RSA modulus.
 * @return                   0:ok, non-zero:error
 */
CC_NONNULL((2, 3, 5, 7, 8, 9))
int ccrsa_sign_pss(ccrsa_full_ctx_t key,
                   const struct ccdigest_info* hashAlgorithm, const struct ccdigest_info* MgfHashAlgorithm,
                   size_t saltSize, struct ccrng_state *rng,
                   size_t hSize, const uint8_t *mHash,
                   size_t *sigSize, uint8_t *sig);

CC_NONNULL((2, 3, 5, 7, 9))
int ccrsa_verify_pss(ccrsa_pub_ctx_t key,
                     const struct ccdigest_info* di, const struct ccdigest_info* MgfDi,
                     size_t digestSize, const uint8_t *digest,
                     size_t sigSize, const uint8_t *sig,
                     size_t saltSize, bool *valid);

/*!
 @function   ccrsa_sign_pkcs1v15
 @abstract   RSA signature with PKCS#1 v1.5 format per PKCS#1 v2.2

 @param      key        Full key
 @param      oid        OID describing the type of digest passed in
 @param      digest_len Byte length of the digest
 @param      digest     Byte array of digest_len bytes containing the digest
 @param      sig_len    Pointer to the number of byte allocate for sig.
                        Output the exact size of the signature.
 @param      sig        Pointer to the allocated buffer of size *sig_len
                        for the output signature

 @result     0 iff successful.
 
  @discussion Null OID is a special case, required to support RFC 4346 where the padding
 is based on SHA1+MD5. In general it is not recommended to use a NULL OID,
 except when strictly required for interoperability

 */
CC_NONNULL((1, 4, 5, 6))
int ccrsa_sign_pkcs1v15(ccrsa_full_ctx_t key, const uint8_t *oid,
                        size_t digest_len, const uint8_t *digest,
                        size_t *sig_len, uint8_t *sig);


/*!
  @function   ccrsa_verify_pkcs1v15
  @abstract   RSA signature with PKCS#1 v1.5 format per PKCS#1 v2.2

  @param      key        Public key
  @param      oid        OID describing the type of digest passed in
  @param      digest_len Byte length of the digest
  @param      digest     Byte array of digest_len bytes containing the digest
  @param      sig_len    Number of byte of the signature sig.
  @param      sig        Pointer to the signature buffer of sig_len
  @param      valid      Output boolean, true if the signature is valid.

  @result     0 iff successful.

  @discussion Null OID is a special case, required to support RFC 4346
  where the padding is based on SHA1+MD5. In general it is not
  recommended to use a NULL OID, except when strictly required for
  interoperability.
*/
CC_NONNULL((1, 4, 6, 7))
int ccrsa_verify_pkcs1v15(ccrsa_pub_ctx_t key, const uint8_t *oid,
                          size_t digest_len, const uint8_t *digest,
                          size_t sig_len, const uint8_t *sig,
                          bool *valid);

/*!
 @function   ccder_encode_rsa_pub_size
 @abstract   Calculate size of public key export format data package.
 
 @param      key        Public key
 
 @result     Returns size required for encoding.
 */

CC_NONNULL((1))
size_t ccder_encode_rsa_pub_size(const ccrsa_pub_ctx_t key);

/*!
 @function   ccrsa_export_priv_pkcs1
 @abstract   Export a public key.
 
 @param      key        Public key
 @param      der        Beginning of output DER buffer
 @param      der_end    End of output DER buffer
 */

CC_NONNULL((1, 2, 3))
uint8_t *ccder_encode_rsa_pub(const ccrsa_pub_ctx_t key, uint8_t *der, uint8_t *der_end);


/*!
 @function   ccder_encode_rsa_priv_size
 @abstract   Calculate size of full key exported in PKCS#1 format.
 
 @param      key        Full key
 
 @result     Returns size required for encoding.
 */

CC_NONNULL((1))
size_t ccder_encode_rsa_priv_size(const ccrsa_full_ctx_t key);

/*!
 @function   ccder_encode_rsa_priv
 @abstract   Export a full key in PKCS#1 format.
 
 @param      key        Full key
 @param      der        Beginning of output DER buffer
 @param      der_end    End of output DER buffer
 */

CC_NONNULL((1, 2, 3))
uint8_t *ccder_encode_rsa_priv(const ccrsa_full_ctx_t key, const uint8_t *der, uint8_t *der_end);

/*!
 @function   ccder_decode_rsa_pub_n
 @abstract   Calculate "n" for a public key imported from a data package.
        PKCS #1 format
 
 @param      der        Beginning of input DER buffer
 @param      der_end    End of input DER buffer
 
 @result the "n" of the RSA key that would result from the import.  This can be used
 to declare the key itself.
 */

CC_NONNULL((1, 2))
cc_size ccder_decode_rsa_pub_n(const uint8_t *der, const uint8_t *der_end);

/*!
 @function   ccder_decode_rsa_pub
 @abstract   Import a public RSA key from a package in public key format.
        PKCS #1 format
 
 @param      key          Public key (n must be set)
 @param      der        Beginning of input DER buffer
 @param      der_end    End of input DER buffer
 
 @result     Key is initialized using the data in the public key message.
 */

CC_NONNULL((1, 2, 3))
const uint8_t *ccder_decode_rsa_pub(const ccrsa_pub_ctx_t key, const uint8_t *der, const uint8_t *der_end);

/*!
 @function   ccder_decode_rsa_pub_x509_n
 @abstract   Calculate "n" for a public key imported from a data package in x509 format

 @param      der        Beginning of input DER buffer
 @param      der_end    End of input DER buffer

 @result the "n" of the RSA key that would result from the import.  This can be used
 to declare the key itself.
 */

CC_NONNULL((1, 2))
cc_size ccder_decode_rsa_pub_x509_n(const uint8_t *der, const uint8_t *der_end);

/*!
 @function   ccder_decode_rsa_pub_x509
 @abstract   Import a public RSA key from a package in x509 format.

 @param      key          Public key (n must be set)
 @param      der        Beginning of input DER buffer
 @param      der_end    End of input DER buffer

 @result     Key is initialized using the data in the public key message.
 */

CC_NONNULL((1, 2, 3))
const uint8_t *ccder_decode_rsa_pub_x509(const ccrsa_pub_ctx_t key, const uint8_t *der, const uint8_t *der_end);


/*!
 @function   ccder_decode_rsa_priv_n
 @abstract   Calculate "n" for a private key imported from a data package.
 
 @param      der        Beginning of input DER buffer
 @param      der_end    End of input DER buffer
 
 @result the "n" of the RSA key that would result from the import.  This can be used
 to declare the key itself.
 */

CC_NONNULL((1, 2))
cc_size ccder_decode_rsa_priv_n(const uint8_t *der, const uint8_t *der_end);

/*!
 @function   ccder_decode_rsa_priv
 @abstract   Import a private RSA key from a package in PKCS#1 format.
 
 @param      key          Full key (n must be set)
 @param      der        Beginning of input DER buffer
 @param      der_end    End of input DER buffer
 
 @result     Key is initialized using the data in the public key message.
 */

CC_NONNULL((1, 2, 3))
const uint8_t *ccder_decode_rsa_priv(const ccrsa_full_ctx_t key, const uint8_t *der, const uint8_t *der_end);

/*!
 @function   ccrsa_export_pub_size
 @abstract   Calculate size of public key exported data package.
 
 @param      key        Public key
 
 @result     Returns size required for encoding.
 */

CC_INLINE CC_NONNULL((1))
size_t ccrsa_export_pub_size(const ccrsa_pub_ctx_t key) {
    return ccder_encode_rsa_pub_size(key);
}

/*!
 @function   ccrsa_export_pub
 @abstract   Export a public key in public key format.
 
 @param      key        Public key
 @param      out_len    Allocated size
 @param      out        Output buffer
 */

CC_NONNULL((1, 3))
int ccrsa_export_pub(const ccrsa_pub_ctx_t key, size_t out_len, uint8_t *out);
/*!
 @function   ccrsa_import_pub_n
 @abstract   Calculate "n" for a public key imported from a data package.
 
 @param      inlen        Length of public key package data
 @param      der          pointer to public key package data
 
 @result the "n" of the RSA key that would result from the import.  This can be used
 to declare the key itself.
 */

CC_INLINE CC_NONNULL((2))
cc_size ccrsa_import_pub_n(size_t inlen, const uint8_t *der) {
    cc_size size = ccder_decode_rsa_pub_x509_n(der, der + inlen);
    if(size == 0) {
        size = ccder_decode_rsa_pub_n(der, der + inlen);
    }
    return size;
}

/*!
 @function   ccrsa_import_pub
 @abstract   Import a public RSA key from a package in public key format.
 
 @param      key          Public key (n must be set)
 @param      inlen        Length of public key package data
 @param      der           pointer to public key package data
 
 @result     Key is initialized using the data in the public key message.
 */

CC_NONNULL((1, 3))
int ccrsa_import_pub(ccrsa_pub_ctx_t key, size_t inlen, const uint8_t *der);

/*!
 @function   ccrsa_export_priv_size
 @abstract   Calculate size of full key exported in PKCS#1 format.
 
 @param      key        Full key
 
 @result     Returns size required for encoding.
 */

CC_INLINE CC_NONNULL((1))
size_t ccrsa_export_priv_size(const ccrsa_full_ctx_t key) {
    return ccder_encode_rsa_priv_size(key);
}

/*!
 @function   ccrsa_export_priv
 @abstract   Export a full key in PKCS#1 format.
 
 @param      key        Full key
 @param      out_len    Allocated size
 @param      out        Output buffer
 */

CC_INLINE CC_NONNULL((1, 3))
int ccrsa_export_priv(const ccrsa_full_ctx_t key, size_t out_len, uint8_t *out) {
    return (ccder_encode_rsa_priv(key, out, out+out_len) != out);
}

/*!
 @function   ccrsa_import_priv_n
 @abstract   Calculate size of full key exported in PKCS#1 format.
 
 @param      inlen        Length of PKCS#1 package data
 @param      der           pointer to PKCS#1 package data
 
 @result the "n" of the RSA key that would result from the import.  This can be used
 to declare the key itself.
 */

CC_INLINE CC_NONNULL((2))
cc_size ccrsa_import_priv_n(size_t inlen, const uint8_t *der) {
    return ccder_decode_rsa_priv_n(der, der + inlen);
}

/*!
 @function   ccrsa_import_priv
 @abstract   Import a full RSA key from a package in PKCS#1 format.
 
 @param      key          Full key (n must be set)
 @param      inlen        Length of PKCS#1 package data
 @param      der           pointer to PKCS#1 package data
 
 @result     Key is initialized using the data in the PKCS#1 message.
 */

CC_INLINE CC_NONNULL((1, 3))
int ccrsa_import_priv(ccrsa_full_ctx_t key, size_t inlen, const uint8_t *der) {
    return (ccder_decode_rsa_priv(key, der, der+inlen) == NULL);
}


CC_NONNULL((1, 2))
int ccrsa_get_pubkey_components(const ccrsa_pub_ctx_t pubkey, uint8_t *modulus, size_t *modulusLength, uint8_t *exponent, size_t *exponentLength);

CC_NONNULL((1, 2))
int ccrsa_get_fullkey_components(const ccrsa_full_ctx_t key, uint8_t *modulus, size_t *modulusLength, uint8_t *exponent, size_t *exponentLength,
                                 uint8_t *p, size_t *pLength, uint8_t *q, size_t *qLength);


/*!
 @function   ccrsa_dump_public_key
 @abstract   Print a rsa public key in the console (printf)

 @param      key          Public key
 */
void ccrsa_dump_public_key(ccrsa_pub_ctx_t key);

/*!
 @function   ccrsa_dump_full_key
 @abstract   Print a rsa private key in the console (printf)

 @param      key          Public key
 */
void ccrsa_dump_full_key(ccrsa_full_ctx_t key);

#endif /* _CORECRYPTO_CCRSA_H_ */
