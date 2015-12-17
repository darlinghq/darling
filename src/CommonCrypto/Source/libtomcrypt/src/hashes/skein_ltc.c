/*
 *  skein_ltc.c
 *  Skein-Test
 *
 *  Created by Jon Callas on 5/25/10.
 *  Copyright 2010 Apple, Inc. All rights reserved.
 *
 */

#include "tomcrypt.h"

#include "skein.h"

#ifdef LTC_SKEIN
#include "skein_ltc.h"

int skein_test(void);

const struct ltc_hash_descriptor skein512_128_desc =
{
    "skein512-128",				/** name of hash **/
    SKEIN512_128_LTC_TAG,			/** internal ID **/
    128/8,						/** Size of digest in octets **/
    64,							/** Input block size in octets */
	
    /* OID */
	{ 1, 2, 840, 113635, 100, 2, 8,  
		128},
	/** Length of DER encoding */
	sizeof("\x06\x0B\x2A\x86\x48\x86\xF7\x63\x64\x02\x08\x81\x00")-1,
	
    &Skein_512_128_Init,
    &Skein_512_Update,
    &Skein_512_Final,
    &skein_test,
    NULL
};

const struct ltc_hash_descriptor skein512_160_desc =
{
    "skein512-160",				/** name of hash **/
    SKEIN512_160_LTC_TAG,			/** internal ID **/
    160/8,						/** Size of digest in octets **/
    64,							/** Input block size in octets */
	
    /* OID */
	{ 1, 2, 840, 113635, 100, 2, 8,  
		160},
	/** Length of DER encoding */
	sizeof("\x06\x0B\x2A\x86\x48\x86\xF7\x63\x64\x02\x08\x81\x20")-1,
	
    &Skein_512_160_Init,
    &Skein_512_Update,
    &Skein_512_Final,
    &skein_test,
    NULL
};

const struct ltc_hash_descriptor skein512_224_desc =
{
    "skein512-224",				/** name of hash **/
    SKEIN512_224_LTC_TAG,			/** internal ID **/
    224/8,						/** Size of digest in octets **/
    64,							/** Input block size in octets */
	
    /* OID */
	{ 1, 2, 840, 113635, 100, 2, 8,  
		224},
	/** Length of DER encoding */
	sizeof("\x06\x0B\x2A\x86\x48\x86\xF7\x63\x64\x02\x08\x81\x60")-1,
	
    &Skein_512_224_Init,
    &Skein_512_Update,
    &Skein_512_Final,
    &skein_test,
    NULL
};

const struct ltc_hash_descriptor skein512_256_desc =
{
    "skein512-256",				/** name of hash **/
    SKEIN512_256_LTC_TAG,			/** internal ID **/
    256/8,						/** Size of digest in octets **/
    64,							/** Input block size in octets */
	
    /* OID */
	{ 1, 2, 840, 113635, 100, 2, 8,  
		256},
	/** Length of DER encoding */
	sizeof("\x06\x0B\x2A\x86\x48\x86\xF7\x63\x64\x02\x08\x82\x00")-1,
	
    &Skein_512_256_Init,
    &Skein_512_Update,
    &Skein_512_Final,
    &skein_test,
    NULL
};

const struct ltc_hash_descriptor skein512_384_desc =
{
    "skein512-384",				/** name of hash **/
    SKEIN512_384_LTC_TAG,			/** internal ID **/
    384/8,						/** Size of digest in octets **/
    64,							/** Input block size in octets */
	
    /* OID */
	{ 1, 2, 840, 113635, 100, 2, 8,  
		384},
	/** Length of DER encoding */
	sizeof("\x06\x0B\x2A\x86\x48\x86\xF7\x63\x64\x02\x08\x83\x00")-1,
	
    &Skein_512_384_Init,
    &Skein_512_Update,
    &Skein_512_Final,
    &skein_test,
    NULL
};

const struct ltc_hash_descriptor skein512_512_desc =
{
    "skein512-512",				/** name of hash **/
    SKEIN512_512_LTC_TAG,			/** internal ID **/
    512/8,						/** Size of digest in octets **/
    64,							/** Input block size in octets */
	
    /* OID */
	{ 1, 2, 840, 113635, 100, 2, 8,  
		512},
	/** Length of DER encoding */
	sizeof("\x06\x0B\x2A\x86\x48\x86\xF7\x63\x64\x02\x08\x84\x00")-1,
	
    &Skein_512_512_Init,
    &Skein_512_Update,
    &Skein_512_Final,
    &skein_test,
    NULL
};

int skein_test(void)
{
#ifndef LTC_TEST
    return CRYPT_NOP;
#else    
	static const struct {
		char *msg;
		unsigned char hash[20];
	} tests[] = {
		{ {0xFF, 0x00 },
		  { 0x42, 0xAA, 0x6B, 0xD9, 0xCA, 0x92, 0xE9, 0x0E, 0xA2, 0x8D, 0xF6, 0xF6, 0xF2, 0xD0, 0xD9, 0xB8, 
			  0x5A, 0x2D, 0x19, 0x07, 0xEE, 0x4D, 0xC1, 0xB1, 0x71, 0xAC, 0xE7, 0xEB, 0x11, 0x59, 0xBE, 0x3B, 
			  0xD1, 0xBC, 0x56, 0x58, 0x6D, 0x92, 0x49, 0x2B, 0x6E, 0xFF, 0x9B, 0xE0, 0x33, 0x06, 0x99, 0x4C, 
			  0x65, 0xA3, 0x32, 0xC4, 0xC2, 0x41, 0x60, 0xF4, 0x66, 0x55, 0x04, 0x0E, 0x55, 0x8E, 0x83, 0x29 }
		  },
		{ { 0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7, 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1, 0xF0,
			0xEF, 0xEE, 0xED, 0xEC, 0xEB, 0xEA, 0xE9, 0xE8, 0xE7, 0xE6, 0xE5, 0xE4, 0xE3, 0xE2, 0xE1, 0xE0,
			0xDF, 0xDE, 0xDD, 0xDC, 0xDB, 0xDA, 0xD9, 0xD8, 0xD7, 0xD6, 0xD5, 0xD4, 0xD3, 0xD2, 0xD1, 0xD0,
			0xCF, 0xCE, 0xCD, 0xCC, 0xCB, 0xCA, 0xC9, 0xC8, 0xC7, 0xC6, 0xC5, 0xC4, 0xC3, 0xC2, 0xC1, 0xC0,
			0x00 },
		  { 0x04, 0xF9, 0x6C, 0x6F, 0x61, 0xB3, 0xE2, 0x37, 0xA4, 0xFA, 0x77, 0x55, 0xEE, 0x4A, 0xCF, 0x34, 
			  0x49, 0x42, 0x22, 0x96, 0x89, 0x54, 0xF4, 0x95, 0xAD, 0x14, 0x7A, 0x1A, 0x71, 0x5F, 0x7A, 0x73, 
			  0xEB, 0xEC, 0xFA, 0x1E, 0xF2, 0x75, 0xBE, 0xD8, 0x7D, 0xC6, 0x0B, 0xD1, 0xA0, 0xBC, 0x60, 0x21, 
			  0x06, 0xFA, 0x98, 0xF8, 0xE7, 0x23, 0x7B, 0xD1, 0xAC, 0x09, 0x58, 0xE7, 0x6D, 0x30, 0x66, 0x78 }
		}
	};
	
	int i;
	unsigned char tmp[64];
	Skein_512_Ctxt_t md;
	
	for (i = 0; i < (int)(sizeof(tests) / sizeof(tests[0]));  i++) {
		Skein_512_Init(&md, 512);
		Skein_512_Update(&md, (unsigned char*)tests[i].msg, (unsigned long)strlen(tests[i].msg));
		Skein_512_Final(&md, tmp);
		if (XMEMCMP(tmp, tests[i].hash, 20) != 0) {
			return CRYPT_FAIL_TESTVECTOR;
		}
	}
	return CRYPT_OK;
#endif
}


#endif
