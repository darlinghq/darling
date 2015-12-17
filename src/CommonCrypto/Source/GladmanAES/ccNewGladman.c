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
 * ccNewGladman.c - shim between Gladman AES and CommonEncryption.
 *
 * Created 3/30/06 by Doug Mitchell. 
 */

#include <CommonCrypto/aesopt.h>

#if CC_AES_USE_HARDWARE
//Need IOKitLib.h only for IOAESTypes.h
#include <IOKit/IOKitLib.h>
#include <Kernel/IOKit/crypto/IOAESTypes.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#endif

/*	Produce object code iff UseGladmanAES is defined.  Otherwise, suppress
	use of this module, because some other AES implementation is being used.
*/
#if defined UseGladmanAES

#ifdef	_APPLE_COMMON_CRYPTO_

#include <strings.h>

int aes_cc_set_key(
	aes_cc_ctx *cx, 
	const void *rawKey, 
	aes_32t keyLength,
	int forEncrypt)
{
	if(forEncrypt) {
		switch(keyLength) {
			case 16: 
				aes_encrypt_key128((const unsigned char *)rawKey, &cx->encrypt);
				break;
			case 24: 
				aes_encrypt_key192((const unsigned char *)rawKey, &cx->encrypt);
				break;
			case 32: 
				aes_encrypt_key256((const unsigned char *)rawKey, &cx->encrypt);
				break;
			default:
				return -1;
		}
		cx->encrypt.cbcEnable = 0;
	}
	else {
		switch(keyLength) {
			case 16: 
				aes_decrypt_key128((const unsigned char *)rawKey, &cx->decrypt);
				break;
			case 24: 
				aes_decrypt_key192((const unsigned char *)rawKey, &cx->decrypt);
				break;
			case 32: 
				aes_decrypt_key256((const unsigned char *)rawKey, &cx->decrypt);
				break;
			default:
				return -1;
		}
		cx->decrypt.cbcEnable = 0;
	}
	return 0;
}

void aes_cc_set_iv(aes_cc_ctx *cx, int forEncrypt, const void *iv)
{
	if(forEncrypt) {
		if(iv == NULL) {
			cx->encrypt.cbcEnable = 0;
		}
		else {
			memmove(cx->encrypt.chainBuf, iv, AES_BLOCK_SIZE);
			cx->encrypt.cbcEnable = 1;
		}
	}
	else {
		if(iv == NULL) {
			cx->decrypt.cbcEnable = 0;
		}
		else {
			memmove(cx->decrypt.chainBuf, iv, AES_BLOCK_SIZE);
			cx->decrypt.cbcEnable = 1;
		}
	}
}

#ifndef	NULL
#define NULL ((void *)0)
#endif

#if CC_AES_USE_HARDWARE
#define CC_AES_USE_HARDWARE_THRESHOLD	(1024 / AES_BLOCK_SIZE) //In Blocks.
static int cc_aes_device = -1;
static pthread_once_t cc_aes_connect_once = PTHREAD_ONCE_INIT;
static aes_32t cc_aes_hardware_quantum = ((256*4096) / AES_BLOCK_SIZE); //In Blocks. Will be set to what hardware returns;

static void cc_aes_connect(void) 
{
	struct IOAESAcceleratorInfo aesInfo;
	
	cc_aes_device = open("/dev/aes_0", O_RDWR, 0);
	if(cc_aes_device < 0)
		return;
	if(ioctl(cc_aes_device, IOAES_GET_INFO, &aesInfo) != -1) {
		cc_aes_hardware_quantum = aesInfo.maxBytesPerCall / AES_BLOCK_SIZE;
	}
}

static aes_32t aes_cc_use_hardware(IOAESOperation operation, int keyLength, UInt8 *key, UInt8 *iv, UInt8* plainText, UInt8 *cipherText, aes_32t numBlocks)
{
	struct IOAESAcceleratorRequest aesRequest;
	aes_32t quantum = cc_aes_hardware_quantum;
	UInt8 *pt = plainText;
	UInt8 *ct = cipherText;
	aes_32t blocks = numBlocks;
	
	aesRequest.operation = operation;
	bcopy(iv, aesRequest.iv.ivBytes, AES_BLOCK_SIZE);
	aesRequest.keyData.key.keyLength = (keyLength << 3); //Hardware needs it in bits.
	bcopy(key, aesRequest.keyData.key.keyBytes, keyLength);

	while (blocks) {
		quantum = ((blocks < cc_aes_hardware_quantum) ? blocks : cc_aes_hardware_quantum);
		aesRequest.plainText = pt;
		aesRequest.cipherText = ct;
		aesRequest.textLength = quantum * AES_BLOCK_SIZE; //The hardware needs textLength in bytes.
		
		if(ioctl(cc_aes_device, IOAES_ENCRYPT_DECRYPT, &aesRequest) == -1) {
			break;
		}
		blocks -= quantum;
		pt += (quantum*AES_BLOCK_SIZE);
		ct += (quantum*AES_BLOCK_SIZE);
	}
	return (numBlocks - blocks);	
} 
#endif

void aes_cc_encrypt(aes_cc_ctx *cx, const void *blocksIn, aes_32t numBlocks, void *blocksOut)
{
	#if CC_AES_USE_HARDWARE
	if(numBlocks > CC_AES_USE_HARDWARE_THRESHOLD && !pthread_once(&cc_aes_connect_once, cc_aes_connect) && cc_aes_device >= 0) {
		aes_encrypt_ctx *cx_encrypt = &cx->encrypt;
		UInt8 *key = cx_encrypt->keyBytes;
		int keyLength = cx_encrypt->keyLength;
		UInt8 *iv = cx_encrypt->chainBuf;
		aes_32t blocks_encrypted = 0;

		blocks_encrypted = aes_cc_use_hardware(IOAESOperationEncrypt, keyLength, key, iv, (UInt8 *)blocksIn, (UInt8 *)blocksOut, numBlocks);
		if(blocks_encrypted == numBlocks) {	// Successfully completed using hardware.
			return;
		}
		//Something went wrong trying to use hardware.. fall through and use the software to do the job.
		blocksIn = (UInt8 *)blocksIn + (blocks_encrypted * AES_BLOCK_SIZE);
		blocksOut = (UInt8 *)blocksOut + (blocks_encrypted * AES_BLOCK_SIZE);
		numBlocks -= blocks_encrypted;
	}
	#endif
	aes_encrypt_cbc((const unsigned char *)blocksIn, 
		NULL,	/* IV - we set via aes_cc_set_iv */
		(unsigned)numBlocks, (unsigned char *)blocksOut, &cx->encrypt);
}

void aes_cc_decrypt(aes_cc_ctx *cx, const void *blocksIn, aes_32t numBlocks,
	void *blocksOut)
{
	#if CC_AES_USE_HARDWARE
	if(numBlocks > CC_AES_USE_HARDWARE_THRESHOLD && !pthread_once(&cc_aes_connect_once, cc_aes_connect) && cc_aes_device >= 0) {
		aes_decrypt_ctx *cx_decrypt = &cx->decrypt;
		UInt8 *key = cx_decrypt->keyBytes;
		int keyLength = cx_decrypt->keyLength;
		UInt8 *iv = cx_decrypt->chainBuf;
		aes_32t blocks_decrypted = 0;

		blocks_decrypted = aes_cc_use_hardware(IOAESOperationDecrypt, keyLength, key, iv, (UInt8 *)blocksOut, (UInt8 *)blocksIn, numBlocks);
		if(blocks_decrypted == numBlocks) {	// Successfully completed using hardware.
			return 1;
		}
		//Something went wrong trying to use hardware.. fall through and use the software to do the job.
		blocksIn = (UInt8 *)blocksIn + (blocks_decrypted * AES_BLOCK_SIZE);
		blocksOut = (UInt8 *)blocksOut + (blocks_decrypted * AES_BLOCK_SIZE);
		numBlocks -= blocks_decrypted;
	}
	#endif
	aes_decrypt_cbc((const unsigned char *)blocksIn,
		NULL,	/* IV - we set via aes_cc_set_iv */
		(unsigned)numBlocks, (unsigned char *)blocksOut, &cx->decrypt);
}

#endif	/* _APPLE_COMMON_CRYPTO_ */

#endif	// defined UseGladmanAES
