/*
 *  ccdes.h
 *  corecrypto
 *
 *  Created on 12/20/2010
 *
 *  Copyright (c) 2010,2012,2015 Apple Inc. All rights reserved.
 *
 */


#ifndef _CORECRYPTO_CCDES_H_
#define _CORECRYPTO_CCDES_H_

#include <corecrypto/ccmode.h>

#define CCDES_BLOCK_SIZE 8
#define CCDES_KEY_SIZE 8

extern const struct ccmode_ecb ccdes3_ltc_ecb_decrypt_mode;
extern const struct ccmode_ecb ccdes3_ltc_ecb_encrypt_mode;

const struct ccmode_ecb *ccdes_ecb_decrypt_mode(void);
const struct ccmode_ecb *ccdes_ecb_encrypt_mode(void);

const struct ccmode_cbc *ccdes_cbc_decrypt_mode(void);
const struct ccmode_cbc *ccdes_cbc_encrypt_mode(void);

const struct ccmode_cfb *ccdes_cfb_decrypt_mode(void);
const struct ccmode_cfb *ccdes_cfb_encrypt_mode(void);

const struct ccmode_cfb8 *ccdes_cfb8_decrypt_mode(void);
const struct ccmode_cfb8 *ccdes_cfb8_encrypt_mode(void);

const struct ccmode_ctr *ccdes_ctr_crypt_mode(void);

const struct ccmode_ofb *ccdes_ofb_crypt_mode(void);


const struct ccmode_ecb *ccdes3_ecb_decrypt_mode(void);
const struct ccmode_ecb *ccdes3_ecb_encrypt_mode(void);

const struct ccmode_cbc *ccdes3_cbc_decrypt_mode(void);
const struct ccmode_cbc *ccdes3_cbc_encrypt_mode(void);

const struct ccmode_cfb *ccdes3_cfb_decrypt_mode(void);
const struct ccmode_cfb *ccdes3_cfb_encrypt_mode(void);

const struct ccmode_cfb8 *ccdes3_cfb8_decrypt_mode(void);
const struct ccmode_cfb8 *ccdes3_cfb8_encrypt_mode(void);

const struct ccmode_ctr *ccdes3_ctr_crypt_mode(void);

const struct ccmode_ofb *ccdes3_ofb_crypt_mode(void);

int ccdes_key_is_weak( void *key, size_t  length);
void ccdes_key_set_odd_parity(void *key, size_t length);

uint32_t
ccdes_cbc_cksum(const void *in, void *out, size_t length,
                const void *key, size_t key_nbytes, const void *ivec);


#endif /* _CORECRYPTO_CCDES_H_ */
