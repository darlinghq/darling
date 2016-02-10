
#ifndef	_RC2_H_
#define _RC2_H_

typedef struct rc2_key_st {
    unsigned short xkey[64];
} RC2_Schedule;

/**********************************************************************\ 
* Expand a variable-length user key (between 1 and 128 bytes) to a     * 
* 64-short working rc2 key, of at most "bits" effective key bits.      * 
* The effective key bits parameter looks like an export control hack.  * 
* For normal use, it should always be set to 1024.  For convenience,   * 
* zero is accepted as an alias for 1024.                               * 
\**********************************************************************/ 
void rc2_keyschedule( RC2_Schedule *key_schedule, 
                      const unsigned char *key, 
                      unsigned len, 
                      unsigned bits );

/**********************************************************************\ 
* Encrypt an 8-byte block of plaintext using the given key.            * 
\**********************************************************************/ 
void rc2_encrypt( const RC2_Schedule *key_schedule, 
                  const unsigned char *plain, 
                  unsigned char *cipher );

/**********************************************************************\ 
* Decrypt an 8-byte block of ciphertext using the given key.           * 
\**********************************************************************/ 
void rc2_decrypt( const RC2_Schedule *key_schedule, 
                  unsigned char *plain, 
                  const unsigned char *cipher );

#endif /* _RC2_H_ */
