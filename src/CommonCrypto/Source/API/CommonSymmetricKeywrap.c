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

#include "CommonSymmetricKeywrap.h"
#include <AssertMacros.h>
#include <stdint.h>
#include <libkern/OSByteOrder.h>


static const uint8_t rfc3394_iv_data[] = { 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6, 0xA6 };

const uint8_t* CCrfc3394_iv = rfc3394_iv_data;
const size_t CCrfc3394_ivLen = sizeof(rfc3394_iv_data);

static uint64_t 
pack64(uint8_t *iv, size_t ivLen)
{
	uint64_t retval;
	int i;
	
	for(i=0, retval=0; i<8; i++) retval = (retval<<8) + iv[i];
	return retval;
}

#if KERNEL

#include <crypto/aes.h>
#include <libkern/OSByteOrder.h>

#define AES128_KEK 1
#define AES192_KEK 0
#define AES256_KEK 0

#define debug kprintf

static bool 
aes_operation(bool encrypt, const uint8_t *kek, size_t kek_len, uint8_t *block)
{
    uint64_t iv[2] = { 0 };

    if (encrypt) {
        aes_encrypt_ctx encrypt_ctx[1];
        switch(kek_len) {
#if AES128_KEK
            case 16: aes_encrypt_key128(kek, encrypt_ctx); break;
#endif
#if AES192_KEK
            case 24: aes_encrypt_key192(kek, encrypt_ctx); break;
#endif
#if AES256_KEK
            case 32: aes_encrypt_key256(kek, encrypt_ctx); break;
#endif
            default: return false;
        }
        aes_encrypt_cbc(block, (uint8_t*)iv, 1, block, encrypt_ctx);
    } else {
        aes_decrypt_ctx decrypt_ctx[1];
        switch(kek_len) {
#if AES128_KEK
            case 16: aes_decrypt_key128(kek, decrypt_ctx); break;
#endif
#if AES192_KEK
            case 24: aes_decrypt_key192(kek, decrypt_ctx); break;
#endif
#if AES256_KEK
            case 32: aes_decrypt_key256(kek, decrypt_ctx); break;
#endif
            default: return false;
        }
        aes_decrypt_cbc(block, (uint8_t*)iv, 1, block, decrypt_ctx);
    }

    return true;
}

#else

#include "CommonCryptor.h"

#define debug printf

static bool 
aes_operation(bool encrypt, const uint8_t *kek, size_t kek_len, uint8_t *block)
{
    uint64_t iv[2] = { 0 };
    size_t bytes_moved = 0;

    return (0 == CCCrypt(encrypt? kCCEncrypt : kCCDecrypt, 
                kCCAlgorithmAES128, 0, kek, kek_len, iv, 
                block, kCCBlockSizeAES128,
                block, kCCBlockSizeAES128, &bytes_moved));
}
#endif


/*
    1) Initialize variables.

    Set A = IV, an initial value (see 2.2.3) 
	For i = 1 to n R[i] = P[i] 

    2) Calculate intermediate values. 

	For j = 0 to 5
		For i=1 to n
			B = AES(K, A | R[i])
			A = MSB(64, B) ^ t where t = (n x j)+i
			R[i] = LSB(64, B)

	3) Output the results. 

			Set C[0] = A 
			For i = 1 to n 
				C[i] = R[i]

    http://www.faqs.org/rfcs/rfc3394.html
	
*/


int  
CCSymmetricKeyWrap( CCWrappingAlgorithm algorithm, 
				   const uint8_t *iv, const size_t ivLen,
				   const uint8_t *kek, size_t kekLen,
				   const uint8_t *rawKey, size_t rawKeyLen,
				   uint8_t  *wrappedKey, size_t *wrappedKeyLen)
{
    uint32_t n = rawKeyLen / 8; /* key size in 64 bit blocks */
    uint64_t (*R)[2]; /* R is a two-dimensional array, with n rows of 2 columns */
    int i, j, err;
	
	// allocate R
	R = calloc(n, sizeof(uint64_t[2])); 
    require_action(R, out, err = -1);
    // don't wrap with something smaller
    require_action(rawKeyLen >= kekLen, out, err = -1);
    // kek multiple of 64 bits: 128, 192, 256
    require_action(kekLen == 16 || kekLen == 24 || kekLen == 32, out, err = -1);
    // wrapped_key_len 64 bits larger than key_len
    require_action(wrappedKeyLen && (*wrappedKeyLen >= rawKeyLen + 64/8), out, err = -1);

    // R[0][1] = P[0] ... R[1][n-1] = P[n-1]
    for (i = 0; i < n; i++)
        memcpy(&R[i][1], rawKey + (64/8) * i, (64/8));

	uint64_t kek_iv = pack64(iv, ivLen);
	
    R[0][0] = kek_iv;

    for (j = 0; j < 6; j++) {
        for (i = 0; i < n; i++)
        {
            require_action(aes_operation(true, kek, kekLen, (uint8_t*)&R[i][0]), out, err = -1);
            R[(i + 1) % n][0] = R[i][0] ^ _OSSwapInt64((n*j)+i+1);
        }
    }
	
    // write output
    memcpy(wrappedKey, &R[0][0], 8);
    for (i = 0; i < n; i++)
        memcpy(wrappedKey + 8 + i * 8, &R[i][1], 8);

    err = 0;
out:
    if (R) free(R);
    return err;
}



int  
CCSymmetricKeyUnwrap( CCWrappingAlgorithm algorithm, 
					 const uint8_t *iv, const size_t ivLen,
					 const uint8_t *kek, size_t kekLen,
					 const uint8_t  *wrappedKey, size_t wrappedKeyLen,
					 uint8_t  *rawKey, size_t *rawKeyLen)
{
    uint32_t n = wrappedKeyLen/8 - 1; /* raw key size in 64 bit blocks */
    uint64_t (*R)[2]; /* R is a two-dimensional array, with n rows of 2 columns */
    int i, j, err;

	// allocate R
	R = calloc(n, sizeof(uint64_t[2])); 
    require_action(R, out, err = -1);
	// kek multiple of 64 bits: 128, 192, 256
    require_action(kekLen == 16 || kekLen == 32, out, err = -1);
    // wrapped_key_len 64 bits larger than key_len
    require_action(rawKeyLen && (*rawKeyLen >= wrappedKeyLen - 64/8), out, err = -1);

    // R[0][1] = C[0] ... R[1][n-1] = C[n-1]
    memcpy(&R[0][0], wrappedKey, 64/8); 
    for (i = 0; i < n; i++)
        memcpy(&R[i][1], wrappedKey + (64/8) * (i+1), 64/8);

    for (j = 5; j >= 0; j--) {
        for (i = n - 1; i >= 0; i--)
        {
           R[i][0] = R[(i + 1) % n][0] ^ _OSSwapInt64((n*j)+i+1);
            require_action(aes_operation(false, kek, kekLen, (uint8_t*)&R[i][0]), out, err = -1);
        }
    }

	uint64_t kek_iv = pack64(iv, ivLen);

    // R[0][0] == iv?
    require_action(R[0][0] == kek_iv, out, err = -1);

    // write output
    for (i = 0; i < n; i++)
        memcpy(rawKey + i * 8, &R[i][1], 8);

    // clean all stack variables

    err = 0;
    
out:
	if (R) free(R);
    return err;
}


size_t
CCSymmetricWrappedSize( CCWrappingAlgorithm algorithm, size_t rawKeyLen)
{
	return (rawKeyLen + 8);
}

size_t
CCSymmetricUnwrappedSize( CCWrappingAlgorithm algorithm, size_t wrappedKeyLen)
{
    return (wrappedKeyLen - 8);
}






