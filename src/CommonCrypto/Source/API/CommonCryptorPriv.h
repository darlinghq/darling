/* 
 * Copyright (c) 2006 Apple Computer, Inc. All Rights Reserved.
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
 * CommonCryptorPriv.h - interface between CommonCryptor and operation- and
 *						 algorithm-specific service providers. 
 */

#ifndef	_CC_COMMON_CRYPTOR_PRIV_
#define	_CC_COMMON_CRYPTOR_PRIV_

#include "CommonCryptor.h"
#include "CommonCryptorSPI.h"
#include "tomcrypt.h"
#include "rc4.h"
#include "pkcs7pad.h"

#ifdef __cplusplus
extern "C" {
#endif
    
static enum {
    ccStreaming		= 0x00000001,
    ccStreamMode	= 0x00000002,
    ccBlockMode		= 0x00000004
};
typedef uint32_t CCBufStrat;
    
#define CCMAXBUFFERSIZE 128
#define CCMAXCONTEXTSIZE 1032
    
static uint32_t ccMaxCipherBlockSize = CCMAXBUFFERSIZE; // rc2/rc5 max blocksize
static uint32_t ccDefaultRounds = 0;

typedef struct _CCCryptor {
    char					buffptr[CCMAXBUFFERSIZE];
    char			 		iv[CCMAXBUFFERSIZE];
    
    CCOperation				op;         // kCCEncrypt, kCCDecrypt, or kCCBoth
    CCAlgorithm				cipher;     // encryption algorithm
    CCMode					mode;       // one of pre-defined modes
    mode_descriptor_ptr		modeptr;
    CCBufStrat				bufStrat;
    CCPadding				padding;    // padding to use 0 (default) or kCCOptionPKCS7Padding
    int32_t					ltcAlgIndex;    // LibTomCrypt cipher index
    uint32_t				blocksize;
    uint32_t				bufferPos;
    uint32_t				bytesProcessed;
    mode_context			*ctx;       // largest size context in use
    void*					mallocAddress;  // if Not NULL, we mallocd this and must free it in CCCryptorRelease()
    uint32_t                modeContext[CCMAXCONTEXTSIZE/4];
} CCCryptor;
static uint32_t cryptorSize = sizeof(struct _CCCryptor);
    
    
typedef struct _CCCompat {
    uint32_t			weMallocd;
    CCCryptor			*cryptor;
} CCCompatCryptor;
    
    
#define CCCRYPTOR_SIZE  sizeof(struct _CCCryptor)
#define kCCContextSizeGENERIC (sizeof(CCCompatCryptor))

    
#ifdef __cplusplus
}
#endif

#endif	/* _CC_COMMON_CRYPTOR_PRIV_ */
