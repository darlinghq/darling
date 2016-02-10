/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtom.org
 */
#include "tomcrypt.h"

/**
  @file cfb_decrypt.c
  CFB implementation, decrypt data, Tom St Denis
*/

#ifdef LTC_CFB_MODE

/**
   CFB decrypt
   @param ct      Ciphertext
   @param pt      [out] Plaintext
   @param len     Length of ciphertext (octets)
   @param cfb     CFB state
   @return CRYPT_OK if successful
*/
int cfb8_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_CFB *cfb)
{
    int err;
    unsigned char *theIV;
    int ivSize;
    
    LTC_ARGCHK(pt != NULL);
    LTC_ARGCHK(ct != NULL);
    LTC_ARGCHK(cfb != NULL);
    
    theIV = (unsigned char *) cfb->IV;
    ivSize = cfb->blocklen;
        
    if ((err = cipher_is_valid(cfb->cipher)) != CRYPT_OK)  return err;
    
    if (cfb->blocklen < 0 || cfb->blocklen > ivSize) return CRYPT_INVALID_ARG;
    
    while (len-- > 0) {
        // XOR the plaintext byte *ct with the leftmost byte of the IV buffer giving *pt
        *pt = cfb->pad[0] ^ *ct;
        // Rotate the IV Buffer left one byte
        memmove(theIV, theIV+1, ivSize-1);
        // copy *ct into the rightmost byte of the IV buffer 
        theIV[ivSize - 1] = *ct;
        // Encrypt the IV buffer
        if ((err = cipher_descriptor[cfb->cipher].ecb_encrypt(cfb->IV, cfb->pad, &cfb->key)) != CRYPT_OK) return err;
        // Bump the pointers
        ++pt; 
        ++ct;
    }
    return CRYPT_OK;
}

#endif


/* $Source: /cvs/libtom/libtomcrypt/src/modes/cfb/cfb_decrypt.c,v $ */
/* $Revision: 1.8 $ */
/* $Date: 2006/12/28 01:27:24 $ */
