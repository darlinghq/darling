/*
 * Copyright (c) 2010 Apple Inc. All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 *  ltc_aes.h
 *  MacTomCrypt
 *
 *  InfoSec Standard Configuration
 *  Copyright 2010 Apple Inc. All rights reserved.
 *
 */

#ifndef _LTC_AES_H_
#define	_LTC_AES_H_

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C"
{
#endif

typedef struct ltc_rijndael_key {
    uint32_t eK[60], dK[60];
    int Nr;
} ltc_rijndael_keysched;

/* make aes an alias */
#define ltc_aes_setup           ltc_rijndael_setup
#define ltc_aes_ecb_encrypt     ltc_rijndael_ecb_encrypt
#define ltc_aes_ecb_decrypt     ltc_rijndael_ecb_decrypt
#define ltc_aes_test            ltc_rijndael_test
#define ltc_aes_done            ltc_rijndael_done
#define ltc_aes_keysize         ltc_rijndael_keysize

#define ltc_aes_enc_setup           ltc_rijndael_enc_setup
#define ltc_aes_enc_ecb_encrypt     ltc_rijndael_enc_ecb_encrypt
#define ltc_aes_enc_keysize         ltc_rijndael_enc_keysize

int ltc_rijndael_setup(const unsigned char *key, int keylen, int num_rounds,
    ltc_rijndael_keysched *skey);
int ltc_rijndael_ecb_encrypt(const unsigned char *pt, unsigned char *ct,
    ltc_rijndael_keysched *skey);
int ltc_rijndael_ecb_decrypt(const unsigned char *ct, unsigned char *pt,
    ltc_rijndael_keysched *skey);
int ltc_rijndael_test(void);
void ltc_rijndael_done(ltc_rijndael_keysched *skey);
int ltc_rijndael_keysize(int *keysize);
int ltc_rijndael_enc_setup(const unsigned char *key, int keylen, int num_rounds,
    ltc_rijndael_keysched *skey);
int ltc_rijndael_enc_ecb_encrypt(const unsigned char *pt, unsigned char *ct,
    ltc_rijndael_keysched *skey);
void ltc_rijndael_enc_done(ltc_rijndael_keysched *skey);
int ltc_rijndael_enc_keysize(int *keysize);
extern const struct ltc_cipher_descriptor ltc_rijndael_desc, ltc_aes_desc;
extern const struct ltc_cipher_descriptor ltc_rijndael_enc_desc,
       ltc_aes_enc_desc;

#if defined(__cplusplus)
}
#endif

#endif /* _LTC_AES_H_ */
