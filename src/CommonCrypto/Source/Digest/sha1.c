/* crypto/sha/sha1.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

/* 
 * In this odd implementation, the actual SHA1 code is in the sha_locl.h header.
 * Compile it exactly once, here.
 */
#define SHA_1   	1

#include <TargetConditionals.h>
#include <CommonCrypto/CommonDigest.h>

#if TARGET_OS_EMBEDDED && __arm__
#define CC_SHA1_USE_HARDWARE			1
#endif

#if CC_SHA1_USE_HARDWARE
#define CC_SHA1_USE_HARDWARE_THRESHOLD	4096
extern int _CC_SHA1_Update(CC_SHA1_CTX *c, const void *data, CC_LONG len);
#endif

#include "sha_locl.h"

#ifdef	_APPLE_COMMON_CRYPTO_

#if CC_SHA1_USE_HARDWARE
//Need the IOKitLib.h only to keep IOSHA1Types.h happy.
#include <IOKit/IOKitLib.h>
#include <Kernel/IOKit/crypto/IOSHA1Types.h>
#include <libkern/OSByteOrder.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>

static int cc_sha1_device = -1;
static pthread_once_t cc_sha1_connect_once = PTHREAD_ONCE_INIT;
static CC_LONG cc_sha1_hardware_quantum = (256*4096); //.25 M default value.

static void cc_sha1_connect(void) {
    struct IOSHA1AcceleratorInfo shaInfo;
	
	cc_sha1_device = open("/dev/sha1_0", O_RDWR, 0);
	if(cc_sha1_device < 0)
		return;

	if(ioctl(cc_sha1_device, IOSHA1_GET_INFO, &shaInfo) != -1) {
        cc_sha1_hardware_quantum = shaInfo.maxBytesPerCall;
	}
}

static CC_LONG sha1_hash_in_hardware(CC_SHA1_CTX *c, const UInt8 *data_buff, CC_LONG length, bool do_final)
{
	// Handle the hardware SHA1. 
	struct IOSHA1AcceleratorRequest shaRequest;
	CC_LONG quantum = cc_sha1_hardware_quantum;
	const UInt8 *data = data_buff;
	CC_LONG bytes_left = length;
	CC_LONG bytes_hashed = 0;
	
	//Load the saved context 
	shaRequest.hashBuffer.hashWords[0] = c->h0;
	shaRequest.hashBuffer.hashWords[1] = c->h1;
	shaRequest.hashBuffer.hashWords[2] = c->h2;
	shaRequest.hashBuffer.hashWords[3] = c->h3;
	shaRequest.hashBuffer.hashWords[4] = c->h4;
    shaRequest.options = 0;

	do {
		if (bytes_left < cc_sha1_hardware_quantum) {
            quantum = bytes_left;
            if (do_final) {
                shaRequest.options = kIOSHA1AcceleratorFinal;
				shaRequest.totalLength = (UInt64)(length) << 3; //Totallength is in bits.
            }
        } else {
            quantum = cc_sha1_hardware_quantum;
        }

		//Split the request in quantums if it is too large.
		shaRequest.sourceText = (UInt8 *)data;
		shaRequest.textLength = quantum;

		if(ioctl(cc_sha1_device, IOSHA1_PERFORM_HASH, &shaRequest) == -1) {
			break; //Failed to complete the whole request but fall back to the software only for the remaining bytes.
		}
		bytes_left -= quantum; 
		data += quantum;
	}while (bytes_left);

	bytes_hashed = (length - bytes_left); 
	if(bytes_hashed) {
		//Save the result in the CC_SHA1_CTX.
		c->h0 = shaRequest.hashBuffer.hashWords[0];
		c->h1 = shaRequest.hashBuffer.hashWords[1];
		c->h2 = shaRequest.hashBuffer.hashWords[2];
		c->h3 = shaRequest.hashBuffer.hashWords[3];
		c->h4 = shaRequest.hashBuffer.hashWords[4];

		//Update Nl and Nh in the context. Required to finish the hash.
		//Copied from the software SHA1 code. 
		CC_LONG l=(c->Nl+(bytes_hashed<<3))&0xffffffffL;
		if (l < c->Nl) /* overflow */
				c->Nh++;
		c->Nh+=(bytes_hashed>>29);
		c->Nl=l;
	}
	return bytes_hashed;
}

int CC_SHA1_Update(CC_SHA1_CTX *c, const void *data, CC_LONG len)
{
	const UInt8 *data_buff = (const UInt8 *) data;
	if (len > CC_SHA1_USE_HARDWARE_THRESHOLD &&
        !(((intptr_t)data_buff + CC_SHA1_BLOCK_BYTES - c->num) & 3) &&
        !pthread_once(&cc_sha1_connect_once, cc_sha1_connect) && cc_sha1_device >= 0) 
    {
		//USE SHA1 hardware.
		if(c->num) {
			//Do the first block or less in software
			CC_LONG partial = CC_SHA1_BLOCK_BYTES - c->num;
			_CC_SHA1_Update(c, data_buff, partial);
			len -= partial;
			data_buff += partial;
		}
		
		CC_LONG bytes_4_hardware = len & ~(CC_SHA1_BLOCK_BYTES - 1); //Send only mulitple of 64 bytes to the hardware.
		CC_LONG bytes_hashed = 0; 
		bytes_hashed = sha1_hash_in_hardware(c, data_buff, bytes_4_hardware, false);
        len -= bytes_hashed;
        data_buff += bytes_hashed;
	}

    //USE SHA1 software. If len is zero then this immediately returns;
    return _CC_SHA1_Update(c, data_buff, len);
}

UInt8* CC_SHA1(const void *data, CC_LONG len, UInt8 *md)
{	
	CC_LONG bytes_hashed = 0;
	const UInt8 *data_buff = (const UInt8 *)data;
	
	if(md == NULL)
		return NULL;									
		
	CC_SHA1_CTX ctx;
	CC_SHA1_Init(&ctx);
	
	if (len > CC_SHA1_USE_HARDWARE_THRESHOLD &&
        !((intptr_t)data_buff & 3) &&
        !pthread_once(&cc_sha1_connect_once, cc_sha1_connect) && cc_sha1_device >= 0) 
    {
		bytes_hashed = sha1_hash_in_hardware(&ctx, data_buff, len, true);
		if (bytes_hashed == len) {
            OSWriteBigInt32(md, 0, ctx.h0);
            OSWriteBigInt32(md, 4, ctx.h1);
            OSWriteBigInt32(md, 8, ctx.h2);
            OSWriteBigInt32(md, 12, ctx.h3);
            OSWriteBigInt32(md, 16, ctx.h4); 
			return md;
        }

		//Either we have failed partially or completely.
		//Fall through to the software.
		data_buff += bytes_hashed;
		len -= bytes_hashed;
	}
	//Fall back to Software SHA1.
	CC_SHA1_Update(&ctx, data_buff, len);
	CC_SHA1_Final(md, &ctx);					
	return md;												
}
#else //#if CC_SHA1_USE_HARDWARE
CC_DIGEST_ONE_SHOT(CC_SHA1, CC_SHA1_CTX, CC_SHA1_Init, CC_SHA1_Update, CC_SHA1_Final)
#endif

#endif //#ifdef	_APPLE_COMMON_CRYPTO_

