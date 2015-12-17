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
 *  aesedpPriv.h
 *  MacTomCrypt
 *
 *  InfoSec Standard Configuration
 *  Copyright 2010 Apple Inc. All rights reserved.
 *
 */

#ifndef AESEDPPRIV_H_
#define AESEDPPRIV_H_
#if defined(__cplusplus)
extern "C"
{
#endif
    
#include "aesedp.h"
// Assembly level interfaces for basic AES.

int 
aes_encrypt_key(const unsigned char *key, int key_len, aesedp_encrypt_ctx cx[1]);

int 
aes_decrypt_key(const unsigned char *key, int key_len, aesedp_decrypt_ctx cx[1]);

int
aes_encrypt(const unsigned char *Plaintext, unsigned char *Ciphertext, aesedp_encrypt_ctx *ctx);

int
aes_decrypt(const unsigned char *Ciphertext, unsigned char *Plaintext, aesedp_decrypt_ctx *ctx);
    
extern int aesedp_decrypt_cbc(const unsigned char *ibuf, const unsigned char *in_iv, unsigned int num_blk,
                              unsigned char *obuf, const aesedp_encrypt_ctx cx[1]);
extern int aesedp_encrypt_cbc(const unsigned char *ibuf, const unsigned char *in_iv, unsigned int num_blk,
                                       unsigned char *obuf, const aesedp_decrypt_ctx ctx[1]);

// MURF ZZZ REMOVE
void AESEncryptCBC(void *Output, const void *Input,
                   void *ChainBuffer, void *Key, long Blocks, long Rounds);
void AESDecryptCBC(void *Output, const void *Input,
                   void *ChainBuffer, void *Key, long Blocks, long Rounds);
    

#if defined(__cplusplus)
}
#endif

#endif /* AESEDPPRIV_H_ */
