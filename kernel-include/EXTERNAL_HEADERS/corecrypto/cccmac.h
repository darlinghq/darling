/*
 *  cccmac.h
 *  corecrypto
 *
 *  Created on 11/07/2013
 *
 *  Copyright (c) 2013,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_cccmac_H_
#define _CORECRYPTO_cccmac_H_

#include <corecrypto/cc.h>
#include <corecrypto/ccmode.h>
#include <corecrypto/ccaes.h>

#define CMAC_BLOCKSIZE   16

struct cccmac_ctx {
    uint8_t k1[CMAC_BLOCKSIZE];
    uint8_t k2[CMAC_BLOCKSIZE];
    uint8_t block[CMAC_BLOCKSIZE];
    size_t  block_nbytes; // Number of byte occupied in block
    size_t  cumulated_nbytes;  // Total size processed
    const struct ccmode_cbc *cbc;
    uint8_t ctx[1];
} CC_ALIGNED(8);// cccmac_ctx_hdr;

typedef struct cccmac_ctx* cccmac_ctx_t;

#define cccmac_hdr_size sizeof(struct cccmac_ctx)


#define cccmac_iv_size(_mode_)  ((_mode_)->block_size)
#define cccmac_cbc_size(_mode_) ((_mode_)->size)

#define cccmac_ctx_size(_mode_) (cccmac_hdr_size + cccmac_iv_size(_mode_) + cccmac_cbc_size(_mode_))
#define cccmac_ctx_n(_mode_)  ccn_nof_size(cccmac_ctx_size(_mode_))

#define cccmac_mode_decl(_mode_, _name_) cc_ctx_decl(struct cccmac_ctx, cccmac_ctx_size(_mode_), _name_)
#define cccmac_mode_clear(_mode_, _name_) cc_clear(cccmac_ctx_size(_mode_), _name_)

/* Return a cccbc_ctx * which can be accesed with the macros in ccmode.h */
#define cccmac_mode_ctx_start(_mode_, HC)    (HC->ctx)
#define CCCMAC_HDR(HC)      (HC)

#define cccmac_mode_sym_ctx(_mode_, HC)     (cccbc_ctx *)(cccmac_mode_ctx_start(_mode_, HC))
#define cccmac_mode_iv(_mode_, HC)     (cccbc_iv *)(cccmac_mode_ctx_start(_mode_, HC)+cccmac_cbc_size(_mode_))
#define cccmac_k1(HC)       (CCCMAC_HDR(HC)->k1)
#define cccmac_k2(HC)       (CCCMAC_HDR(HC)->k2)
#define cccmac_block(HC)    (CCCMAC_HDR(HC)->block)
#define cccmac_cbc(HC)      (CCCMAC_HDR(HC)->cbc)
#define cccmac_block_nbytes(HC)        (CCCMAC_HDR(HC)->block_nbytes)
#define cccmac_cumulated_nbytes(HC)    (CCCMAC_HDR(HC)->cumulated_nbytes)


/* CMAC as defined in NIST SP800-38B - 2005 */

/* =============================================================================

                                ONE SHOT

 ==============================================================================*/

/*!
 @function   cccmac_one_shot_generate
 @abstract   CMAC generation in one call

 @param   cbc          CBC and block cipher specification
 @param   key_nbytes   Length of the key in bytes
 @param   key          Pointer to the key of length key_nbytes
 @param   data_nbytes  Length of the data in bytes
 @param   data         Pointer to the data in bytes
 @param   mac_nbytes   Length in byte of the mac, > 0
 @param   mac          Output of length cbc->block_size

 @result     0 iff successful.

 @discussion Only supports CMAC_BLOCKSIZE block ciphers
 */
int cccmac_one_shot_generate(const struct ccmode_cbc *cbc,
                        size_t key_nbytes, const void *key,
                        size_t data_nbytes, const void *data,
                        size_t mac_nbytes, void *mac);

/*!
 @function   cccmac_one_shot_verify
 @abstract   CMAC verification in one call

 @param   cbc          CBC and block cipher specification
 @param   key_nbytes  Length of the key in bytes
 @param   key          Pointer to the key of length key_nbytes
 @param   data_nbytes Length of the data in bytes
 @param   data         Pointer to the data in bytes
 @param   expected_mac_nbytes  Length in byte of the mac, > 0
 @param   expected_mac Mac value expected

 @result     0 iff successful.

 @discussion Only supports CMAC_BLOCKSIZE block ciphers
 */
int cccmac_one_shot_verify(const struct ccmode_cbc *cbc,
                           size_t key_nbytes, const void *key,
                           size_t data_nbytes, const void *data,
                           size_t expected_mac_nbytes, const void *expected_mac);

/* =============================================================================

                               STREAMING
 
                        Init - Update - Final

==============================================================================*/

/*!
 @function   cccmac_init
 @abstract   Init CMAC context with CBC mode and key

 @param   cbc         CBC and block cipher specification
 @param   ctx         Context use to store internal state
 @param   key_nbytes  Length of the key in bytes
 @param   key         Full key

 @result     0 iff successful.

 @discussion Only supports CMAC_BLOCKSIZE block ciphers
 */

int cccmac_init(const struct ccmode_cbc *cbc,
                cccmac_ctx_t ctx,
                size_t key_nbytes, const void *key);

/*!
 @function   cccmac_update
 @abstract   Process data

 @param   ctx          Context use to store internal state
 @param   data_nbytes Length in byte of the data
 @param   data         Data to process

 @result     0 iff successful.

 @discussion Only supports CMAC_BLOCKSIZE block ciphers
 */

int cccmac_update(cccmac_ctx_t ctx,
                  size_t data_nbytes, const void *data);

/*!
 @function   cccmac_final_generate
 @abstract   Final step for generation

 @param   ctx          Context use to store internal state
 @param   mac_nbytes   Length in byte of the mac, > 0
 @param   mac          Output of length mac_nbytes

 @result     0 iff successful.

 @discussion Only supports CMAC_BLOCKSIZE block ciphers
 */
int cccmac_final_generate(cccmac_ctx_t ctx,
                     size_t mac_nbytes, void *mac);

/*!
 @function   cccmac_final_verify
 @abstract   Final step and verification

 @param   ctx          Context use to store internal state
 @param   expected_mac_nbytes  Length in byte of the mac, > 0
 @param   expected_mac Mac value expected

 @result     0 iff successful.

 @discussion Only supports CMAC_BLOCKSIZE block ciphers
 */
int cccmac_final_verify(cccmac_ctx_t ctx,
                        size_t expected_mac_nbytes, const void *expected_mac);

#endif /* _CORECRYPTO_cccmac_H_ */
