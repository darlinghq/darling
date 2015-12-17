/*
 *  aesedp.c
 *  MacTomCrypt
 *
 *  InfoSec Standard Configuration
 *  Copyright 2010 Apple Inc. All rights reserved.
 *
 */


#include "tomcrypt.h"
#include "aesedpPriv.h"
#include "aesxts.h"
#include "ccdebug.h"
const struct ltc_cipher_descriptor aesedp_desc =
{
    "aesedp", // Name
    65,       // Internal ID
    16, 32, 16, 10, // Min KeySize, Max Keysize, Block Size, Rounds
    aesedp_setup, aesedp_ecb_encrypt, aesedp_ecb_decrypt, aesedp_test, aesedp_done, aesedp_keysize,
    /* ECB Accelerators */
    NULL, NULL, 
    /* CBC Accelerators */
#if defined (__i386__) || defined (__x86_64__)
    aesedp_cbc_encrypt, aesedp_cbc_decrypt,
#else
    NULL, NULL,
#endif
    /* CTR Accelerator */
    NULL, 
    /* LRW Accelerators */
    NULL, NULL,
#if defined (__i386__) || defined (__x86_64__)
    /* XTS Accelerators */
    aesxts_encrypt, aesxts_decrypt,
#else
    NULL, NULL,
#endif
    /* CCM Accelerator */
    NULL, 
    /* GCM Accelerator */
    NULL, 
    /* OMAC Accelerator */
    NULL,
    /* XCBC Accelerator */
    NULL, 
    /* F9 Accelerator */
    NULL
};


int 
aesedp_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey)
{
#if defined (__i386__) || defined (__x86_64__)

	aesedp_ctx *ctx = &skey->aesedp;
	int retval;

	if((retval = aesedp_keysize(&keylen)) != CRYPT_OK) return retval;
	
	if((retval = aes_encrypt_key(key, keylen, &ctx->encrypt)) != CRYPT_OK) return CRYPT_ERROR;
	if((retval = aes_decrypt_key(key, keylen, &ctx->decrypt)) != CRYPT_OK) return CRYPT_ERROR;
	return CRYPT_OK;

#else
	return CRYPT_UNIMPLEMENTED;
#endif
}


int 
aesedp_ecb_encrypt(const unsigned char *pt, unsigned char *ct, symmetric_key *skey)
{
#if defined (__i386__) || defined (__x86_64__)

	aesedp_ctx *ctx = &skey->aesedp;
	return aes_encrypt(pt, ct, &ctx->encrypt);

#else
	return CRYPT_UNIMPLEMENTED;
#endif
}



int 
aesedp_ecb_decrypt(const unsigned char *ct, unsigned char *pt, symmetric_key *skey)
{
#if defined (__i386__) || defined (__x86_64__)

	aesedp_ctx *ctx = &skey->aesedp;
    return aes_decrypt(ct, pt, &ctx->decrypt);

#else
	return CRYPT_UNIMPLEMENTED;
#endif
}



int 
aesedp_test(void)
{
#if defined (__i386__) || defined (__x86_64__)
 #ifndef LTC_TEST
    return CRYPT_NOP;
 #else    
 int err;
 static const struct {
     int keylen;
     unsigned char key[32], pt[16], ct[16];
 } tests[] = {
    { 16,
      { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f }, 
      { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
      { 0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30, 
        0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a }
    }, { 
      24,
      { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 },
      { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
      { 0xdd, 0xa9, 0x7c, 0xa4, 0x86, 0x4c, 0xdf, 0xe0, 
        0x6e, 0xaf, 0x70, 0xa0, 0xec, 0x0d, 0x71, 0x91 }
    }, {
      32,
      { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f },
      { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
      { 0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf, 
        0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89 }
    }
 };
 
 symmetric_key key;
 unsigned char tmp[2][16];
 int i, y;
 
 for (i = 0; i < (int)(sizeof(tests)/sizeof(tests[0])); i++) {
    zeromem(&key, sizeof(key));
    if ((err = aesedp_setup(tests[i].key, tests[i].keylen, 0, &key)) != CRYPT_OK) { 
       return err;
    }
  
    aesedp_ecb_encrypt(tests[i].pt, tmp[0], &key);
    aesedp_ecb_decrypt(tmp[0], tmp[1], &key);
    if (XMEMCMP(tmp[0], tests[i].ct, 16) || XMEMCMP(tmp[1], tests[i].pt, 16)) { 
#if 0
       printf("\n\nTest %d failed\n", i);
       if (XMEMCMP(tmp[0], tests[i].ct, 16)) {
          printf("CT: ");
          for (i = 0; i < 16; i++) {
             printf("%02x ", tmp[0][i]);
          }
          printf("\n");
       } else {
          printf("PT: ");
          for (i = 0; i < 16; i++) {
             printf("%02x ", tmp[1][i]);
          }
          printf("\n");
       }
#endif       
        return CRYPT_FAIL_TESTVECTOR;
    }

      /* now see if we can encrypt all zero bytes 1000 times, decrypt and come back where we started */
      for (y = 0; y < 16; y++) tmp[0][y] = 0;
      for (y = 0; y < 1000; y++) aesedp_ecb_encrypt(tmp[0], tmp[0], &key);
      for (y = 0; y < 1000; y++) aesedp_ecb_decrypt(tmp[0], tmp[0], &key);
      for (y = 0; y < 16; y++) if (tmp[0][y] != 0) return CRYPT_FAIL_TESTVECTOR;
 }       
 return CRYPT_OK;
 #endif
#else
	return CRYPT_UNIMPLEMENTED;
#endif

}



void 
aesedp_done(symmetric_key *skey)
{
}


int 
aesedp_cbc_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long blocks, unsigned char *iv, symmetric_key *skey)
{
    aesedp_ctx *ctx = &skey->aesedp;
    return aesedp_encrypt_cbc((const unsigned char *) pt, (const unsigned char *) iv, blocks, ct, &ctx->encrypt);
}

int 
aesedp_cbc_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long blocks, unsigned char *iv, symmetric_key *skey)
{
    aesedp_ctx *ctx = &skey->aesedp;
    return aesedp_decrypt_cbc((const unsigned char *) ct, (const unsigned char *) iv, blocks, pt, &ctx->decrypt);
}


int 
aesedp_keysize(int *keysize)
{
#if defined (__i386__) || defined (__x86_64__)
	switch (*keysize) {
		case 16:
		case 24:
		case 32:
			return CRYPT_OK;
		default:
			return CRYPT_INVALID_KEYSIZE;
	}
#else
	return CRYPT_UNIMPLEMENTED;
#endif
}

