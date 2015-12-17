/* 
 * Copyright (c) 2011 Apple Computer, Inc. All Rights Reserved.
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

#include "CommonCMACSPI.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonHMAC.h>

/* Internal functions to support one-shot */

uint8_t const_Rb[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87
};

void leftshift_onebit(uint8_t *input, uint8_t *output)
{
    int		i;
    uint8_t	overflow = 0;
    
    for ( i=15; i>=0; i-- ) {
        output[i] = input[i] << 1;
        output[i] |= overflow;
        overflow = (input[i] & 0x80)?1:0;
    }
    return;
}

void xor_128(const uint8_t *a, const uint8_t *b, uint8_t *out)
{
    int i;
    for (i=0;i<16; i++) out[i] = a[i] ^ b[i];
}


void ccGenAESSubKey(const void *key, void *key1, void *key2)
{
    uint8_t L[16];
    uint8_t Z[16];
    uint8_t tmp[16];
    size_t	moved = 0;
    
	memset(Z, 0, 16);
    
	CCCrypt(kCCEncrypt, kCCAlgorithmAES128, kCCOptionECBMode, key, 16, NULL, Z, 16, L, 16, &moved);
    
    if ( (L[0] & 0x80) == 0 ) { /* If MSB(L) = 0, then K1 = L << 1 */
        leftshift_onebit(L, key1);
    } else {    /* Else K1 = ( L << 1 ) (+) Rb */
        leftshift_onebit(L, tmp);
        xor_128(tmp,const_Rb, key1);
    }
    
    if ( (((uint8_t *)key1)[0] & 0x80) == 0 ) {
        leftshift_onebit(key1, key2);
    } else {
        leftshift_onebit(key1, tmp);
        xor_128(tmp,const_Rb, key2);
    }
    return;
    
}

void ccAESCMacPadding (const uint8_t *lastb, uint8_t *pad, int length)
{
    int         j;
    
    for ( j=0; j<16; j++ ) {
        if ( j < length ) pad[j] = lastb[j];
        else if ( j == length ) pad[j] = 0x80;
        else pad[j] = 0x00;
    }
}

/* This would be the one-shot CMAC interface */

void CCAESCmac(const void *key,
               const uint8_t *data,
               size_t dataLength,			/* length of data in bytes */
               void *macOut)				/* MAC written here */
{
    uint8_t       X[16],Y[16], M_last[16], padded[16];
    uint8_t       K1[16], K2[16];
    int         n, i, flag;
    size_t		moved = 0;
    
	// CMacInit
    
    ccGenAESSubKey(key,K1,K2);
    
    // CMacUpdates (all in this case)
    
    n = (dataLength+15) / 16;       /* n is number of rounds */
    
    if ( 0 == n ) {
        n = 1;
        flag = 0;
    } else {
        if ( (dataLength%16) == 0 ) flag = 1;
        else  flag = 0;
    }
    
    if ( flag ) { /* last block is complete block */
        xor_128(&data[16*(n-1)],K1,M_last);
    } else {
        ccAESCMacPadding(&data[16*(n-1)],padded,dataLength%16);
        xor_128(padded,K2,M_last);
    }
    
    memset(X, 0, 16);
    for ( i=0; i<n-1; i++ ) {
        xor_128(X,&data[16*i],Y); /* Y := Mi (+) X  */
        CCCrypt(kCCEncrypt, kCCAlgorithmAES128, kCCOptionECBMode, key, 16, NULL, Y, 16, X, 16, &moved);
    }
    
    // CMacFinal
    
    xor_128(X,M_last,Y);
	CCCrypt(kCCEncrypt, kCCAlgorithmAES128, kCCOptionECBMode, key, 16, NULL, Y, 16, X, 16, &moved);
    
    memcpy(macOut, X, 16);
}
