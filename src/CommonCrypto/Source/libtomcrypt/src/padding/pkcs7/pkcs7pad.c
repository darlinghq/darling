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
 *  pkcs7pad.c
 *  MacTomCrypt
 *
 * PKCS7 padding mode functions.
 */


#include "pkcs7pad.h"

#define MAXBLOCKSIZE_PKCS7 128

int
pkcs7_pad(char *buff, size_t blocksize, size_t startpoint) {
	char padbyte;
	int  i;

	if((buff == NULL) || (blocksize > MAXBLOCKSIZE_PKCS7) || (startpoint > blocksize)) return -1;
	padbyte = blocksize - startpoint;
	if(padbyte == 0) padbyte = blocksize;

	for(i = 0; i < padbyte; i++) buff[startpoint + i] = padbyte;
	return padbyte;
}


int
pkcs7_len(size_t blocksize) {
    if(blocksize > MAXBLOCKSIZE_PKCS7) return -1;
    return blocksize * 2;
}

int
pkcs7_unpadlen(char *buff, size_t blocksize) {
    if(blocksize > MAXBLOCKSIZE_PKCS7) return -1;
	 if(buff == NULL) return -1;
    return blocksize - buff[blocksize - 1];
}


