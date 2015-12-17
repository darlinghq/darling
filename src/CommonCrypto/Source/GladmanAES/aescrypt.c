/*
 ---------------------------------------------------------------------------
 Copyright (c) 2003, Dr Brian Gladman, Worcester, UK.   All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products
      built using this software without specific written permission.

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue 28/01/2004

 This file contains the code for implementing encryption and decryption
 for AES (Rijndael) for block and key sizes of 16, 24 and 32 bytes. It
 can optionally be replaced by code written in assembler using NASM. For
 further details see the file aesopt.h
*/

#include <CommonCrypto/aesopt.h>
#include "aestab.h"
#include <strings.h>

/*	Produce object code iff UseGladmanAES is defined.  Otherwise, suppress
	use of this module, because some other AES implementation is being used.
*/
#if defined UseGladmanAES

#if defined(__cplusplus)
extern "C"
{
#endif

#define ki(y,x,k,c) (s(y,c) = s(x, c) ^ (k)[c])
#define xo(y,x,c) (s(y,c) ^= s(x, c))
#define si(y,x,c)   (s(y,c) = word_in(x, c))
#define so(y,x,c)   word_out(y, c, s(x,c))

#if defined(ARRAYS)
#define locals(y,x)     x[4],y[4]
#else
#define locals(y,x)     x##0,x##1,x##2,x##3,y##0,y##1,y##2,y##3
#endif

#define dtables(tab)     const aes_32t *tab##0, *tab##1, *tab##2, *tab##3
#define itables(tab)     tab##0 = tab[0]; tab##1 = tab[1]; tab##2 = tab[2]; tab##3 = tab[3]

#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1); \
                        s(y,2) = s(x,2); s(y,3) = s(x,3);

#define key_in(y,x,k)   ki(y,x,k,0); ki(y,x,k,1); ki(y,x,k,2); ki(y,x,k,3)
#define cbc(y,x)        xo(y,x,0); xo(y,x,1); xo(y,x,2); xo(y,x,3)
#define state_in(y,x)   si(y,x,0); si(y,x,1); si(y,x,2); si(y,x,3)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); so(y,x,3)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); rm(y,x,k,3)

#if defined(ENCRYPTION) && !defined(AES_ASM)

/* Visual C++ .Net v7.1 provides the fastest encryption code when using
   Pentium optimiation with small code but this is poor for decryption
   so we need to control this with the following VC++ pragmas
*/

#if defined(_MSC_VER)
#pragma optimize( "s", on )
#endif

/* Given the column (c) of the output state variable, the following
   macros give the input state variables which are needed in its
   computation for each row (r) of the state. All the alternative
   macros give the same end values but expand into different ways
   of calculating these values.  In particular the complex macro
   used for dynamically variable block sizes is designed to expand
   to a compile time constant whenever possible but will expand to
   conditional clauses on some branches (I am grateful to Frank
   Yellin for this construction)
*/

#define fwd_var(x,r,c)\
 ( r == 0 ? ( c == 0 ? s(x,0) : c == 1 ? s(x,1) : c == 2 ? s(x,2) : s(x,3))\
 : r == 1 ? ( c == 0 ? s(x,1) : c == 1 ? s(x,2) : c == 2 ? s(x,3) : s(x,0))\
 : r == 2 ? ( c == 0 ? s(x,2) : c == 1 ? s(x,3) : c == 2 ? s(x,0) : s(x,1))\
 :          ( c == 0 ? s(x,3) : c == 1 ? s(x,0) : c == 2 ? s(x,1) : s(x,2)))

#if defined(FT4_SET)
#undef  dec_fmvars
#  if defined(ENC_ROUND_CACHE_TABLES)
#define fwd_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ four_cached_tables(x,t_fn,fwd_var,rf1,c))
#  else
#define fwd_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ four_tables(x,t_fn,fwd_var,rf1,c))
#  endif
#elif defined(FT1_SET)
#undef  dec_fmvars
#define fwd_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ one_table(x,upr,t_fn,fwd_var,rf1,c))
#else
#define fwd_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ fwd_mcol(no_table(x,t_sbox,fwd_var,rf1,c)))
#endif

#if defined(FL4_SET)
#  if defined(LAST_ENC_ROUND_CACHE_TABLES)
#define fwd_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ four_cached_tables(x,t_fl,fwd_var,rf1,c))
#  else
#define fwd_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ four_tables(x,t_fl,fwd_var,rf1,c))
#  endif
#elif defined(FL1_SET)
#define fwd_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ one_table(x,ups,t_fl,fwd_var,rf1,c))
#else
#define fwd_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ no_table(x,t_sbox,fwd_var,rf1,c))
#endif

aes_rval aes_encrypt_cbc(const unsigned char *in, const unsigned char *in_iv, unsigned int num_blk,
					 unsigned char *out, aes_encrypt_ctx cx[1])
{   aes_32t         locals(b0, b1);
    const aes_32t   *kp;
    const aes_32t   *kptr = cx->ks;
	#ifdef _APPLE_COMMON_CRYPTO_
	int				cbcEnable = (cx->cbcEnable || in_iv) ? 1 : 0;
	#endif
	
#if defined(ENC_ROUND_CACHE_TABLES)
	dtables(t_fn);
#endif
#if defined(LAST_ENC_ROUND_CACHE_TABLES)
	dtables(t_fl);
#endif

#if defined( dec_fmvars )
    dec_fmvars; /* declare variables for fwd_mcol() if needed */
#endif

#if defined( AES_ERR_CHK )
    if( cx->rn != 10 && cx->rn != 12 && cx->rn != 14 )
        return aes_error;
#endif

	// Load IV into b0.
	#ifdef		_APPLE_COMMON_CRYPTO_
	if(in_iv) {
		state_in(b0, in_iv);
	}
	else {
		state_in(b0, cx->chainBuf);
	}
	#else
	state_in(b0, in_iv);
	#endif	/* _APPLE_COMMON_CRYPTO_ */
	
	for (;num_blk; in += AES_BLOCK_SIZE, out += AES_BLOCK_SIZE, --num_blk)
	{
		kp = kptr;
#if 0
		// Read the plaintext into b1
		state_in(b1, in);
		#ifdef _APPLE_COMMON_CRYPTO_
		if(cbcEnable) {
		#endif
			// Do the CBC with b0 which is either the iv or the ciphertext of the 
			// previous block.
			cbc(b1, b0);
		#ifdef _APPLE_COMMON_CRYPTO_
		}
		#endif

		// Xor b1 with the key schedule to get things started.
		key_in(b0, b1, kp);
#else
		#ifdef _APPLE_COMMON_CRYPTO_
		if(cbcEnable) {
		#endif
			// Since xor is associative we mess with the ordering here to get 
			// the loads started early
			key_in(b1, b0, kp);  // Xor b0(IV) with the key schedule and assign to b1
			state_in(b0, in);    // Load block into b0
			cbc(b0, b1);         // Xor b0 with b1 and store in b0
		#ifdef _APPLE_COMMON_CRYPTO_
		}
		else {
			// Read the plaintext into b1
			state_in(b1, in);
			key_in(b0, b1, kp);
		}
		#endif	/* _APPLE_COMMON_CRYPTO_ */
#endif	/* 0 */

#if defined(ENC_ROUND_CACHE_TABLES)
		itables(t_fn);
#endif

#if (ENC_UNROLL == FULL)

		switch(cx->rn)
		{
		case 14:
			round(fwd_rnd,  b1, b0, kp + 1 * N_COLS);
			round(fwd_rnd,  b0, b1, kp + 2 * N_COLS);
			kp += 2 * N_COLS;
		case 12:
			round(fwd_rnd,  b1, b0, kp + 1 * N_COLS);
			round(fwd_rnd,  b0, b1, kp + 2 * N_COLS);
			kp += 2 * N_COLS;
		case 10:
		default:
			round(fwd_rnd,  b1, b0, kp + 1 * N_COLS);
			round(fwd_rnd,  b0, b1, kp + 2 * N_COLS);
			round(fwd_rnd,  b1, b0, kp + 3 * N_COLS);
			round(fwd_rnd,  b0, b1, kp + 4 * N_COLS);
			round(fwd_rnd,  b1, b0, kp + 5 * N_COLS);
			round(fwd_rnd,  b0, b1, kp + 6 * N_COLS);
			round(fwd_rnd,  b1, b0, kp + 7 * N_COLS);
			round(fwd_rnd,  b0, b1, kp + 8 * N_COLS);
			round(fwd_rnd,  b1, b0, kp + 9 * N_COLS);
#if defined(LAST_ENC_ROUND_CACHE_TABLES)
			itables(t_fl);
#endif
			round(fwd_lrnd, b0, b1, kp +10 * N_COLS);
		}

#else

		{   aes_32t    rnd;
#if (ENC_UNROLL == PARTIAL)
			for(rnd = 0; rnd < (cx->rn >> 1) - 1; ++rnd)
			{
				kp += N_COLS;
				round(fwd_rnd, b1, b0, kp);
				kp += N_COLS;
				round(fwd_rnd, b0, b1, kp);
			}
			kp += N_COLS;
			round(fwd_rnd,  b1, b0, kp);
#else
			for(rnd = 0; rnd < cx->rn - 1; ++rnd)
			{
				kp += N_COLS;
				round(fwd_rnd, b1, b0, kp);
				l_copy(b0, b1);
			}
#endif
#if defined(LAST_ENC_ROUND_CACHE_TABLES)
			itables(t_fl);
#endif
			kp += N_COLS;
			round(fwd_lrnd, b0, b1, kp);
		}
#endif
	
		state_out(out, b0);
	}
	
	#ifdef _APPLE_COMMON_CRYPTO_
	if(cbcEnable) {
		state_out(cx->chainBuf, b0);
	}
	#endif

#if defined( AES_ERR_CHK )
    return aes_good;
#endif
}

#endif

#if defined(DECRYPTION) && !defined(AES_ASM)

/* Visual C++ .Net v7.1 provides the fastest encryption code when using
   Pentium optimiation with small code but this is poor for decryption
   so we need to control this with the following VC++ pragmas
*/

#if defined(_MSC_VER)
#pragma optimize( "t", on )
#endif

/* Given the column (c) of the output state variable, the following
   macros give the input state variables which are needed in its
   computation for each row (r) of the state. All the alternative
   macros give the same end values but expand into different ways
   of calculating these values.  In particular the complex macro
   used for dynamically variable block sizes is designed to expand
   to a compile time constant whenever possible but will expand to
   conditional clauses on some branches (I am grateful to Frank
   Yellin for this construction)
*/

#define inv_var(x,r,c)\
 ( r == 0 ? ( c == 0 ? s(x,0) : c == 1 ? s(x,1) : c == 2 ? s(x,2) : s(x,3))\
 : r == 1 ? ( c == 0 ? s(x,3) : c == 1 ? s(x,0) : c == 2 ? s(x,1) : s(x,2))\
 : r == 2 ? ( c == 0 ? s(x,2) : c == 1 ? s(x,3) : c == 2 ? s(x,0) : s(x,1))\
 :          ( c == 0 ? s(x,1) : c == 1 ? s(x,2) : c == 2 ? s(x,3) : s(x,0)))

#if defined(IT4_SET)
#undef  dec_imvars
#  if defined(DEC_ROUND_CACHE_TABLES)
#define inv_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ four_cached_tables(x,t_in,inv_var,rf1,c))
#  else
#define inv_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ four_tables(x,t_in,inv_var,rf1,c))
#  endif
#elif defined(IT1_SET)
#undef  dec_imvars
#define inv_rnd(y,x,k,c)    (s(y,c) = (k)[c] ^ one_table(x,upr,t_in,inv_var,rf1,c))
#else
#define inv_rnd(y,x,k,c)    (s(y,c) = inv_mcol((k)[c] ^ no_table(x,t_ibox,inv_var,rf1,c)))
#endif

#if defined(IL4_SET)
#  if defined(LAST_DEC_ROUND_CACHE_TABLES)
#define inv_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ four_cached_tables(x,t_il,inv_var,rf1,c))
#  else
#define inv_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ four_tables(x,t_il,inv_var,rf1,c))
#  endif
#elif defined(IL1_SET)
#define inv_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ one_table(x,ups,t_il,inv_var,rf1,c))
#else
#define inv_lrnd(y,x,k,c)   (s(y,c) = (k)[c] ^ no_table(x,t_ibox,inv_var,rf1,c))
#endif


aes_rval aes_decrypt_cbc(const unsigned char *in, const unsigned char *in_iv, unsigned int num_blk,
					 unsigned char *out, aes_decrypt_ctx cx[1])
{   aes_32t        locals(b0, b1);
    const aes_32t *kptr = cx->ks + cx->rn * N_COLS;
	const aes_32t *kp;
#if defined(DEC_ROUND_CACHE_TABLES)
	dtables(t_in);
#endif
#if defined(LAST_DEC_ROUND_CACHE_TABLES)
	dtables(t_il);
#endif
	#ifdef		_APPLE_COMMON_CRYPTO_
	int cbcEnable = (cx->cbcEnable || in_iv) ? 1 : 0;
	unsigned char lastIv[AES_BLOCK_SIZE];
	
	/* fix a compiler warning... */
	b00 = 0; b01 = 0; b02=0; b03 = 0;
	#endif
	
#if defined( dec_imvars )
    dec_imvars; /* declare variables for inv_mcol() if needed */
#endif
	
#if defined( AES_ERR_CHK )
    if( cx->rn != 10 && cx->rn != 12 && cx->rn != 14 )
        return aes_error;
#endif

#if defined(DEC_ROUND_CACHE_TABLES)
	itables(t_in);
#endif	
	
	in += AES_BLOCK_SIZE * (num_blk - 1);
	out += AES_BLOCK_SIZE * (num_blk - 1);
	// Load the last block's ciphertext into b1
	state_in(b1, in);

	#ifdef	_APPLE_COMMON_CRYPTO_
	/* save that last ciphertext block for next op's chain */
	if(cbcEnable & (num_blk != 0)) {
		memmove(lastIv, in, AES_BLOCK_SIZE);
	}
	#endif
	
	for (;num_blk; out -= AES_BLOCK_SIZE, --num_blk)
	{
		kp = kptr;
		// Do the xor part of state_in, where b1 is the previous block's ciphertext.
		key_in(b0, b1, kp);

#if (DEC_UNROLL == FULL)
	
		switch(cx->rn)
		{
		case 14:
			round(inv_rnd,  b1, b0, kp -  1 * N_COLS);
			round(inv_rnd,  b0, b1, kp -  2 * N_COLS);
			kp -= 2 * N_COLS;
		case 12:
			round(inv_rnd,  b1, b0, kp -  1 * N_COLS);
			round(inv_rnd,  b0, b1, kp -  2 * N_COLS);
			kp -= 2 * N_COLS;
		case 10:
		default:
			round(inv_rnd,  b1, b0, kp -  1 * N_COLS);
			round(inv_rnd,  b0, b1, kp -  2 * N_COLS);
			round(inv_rnd,  b1, b0, kp -  3 * N_COLS);
			round(inv_rnd,  b0, b1, kp -  4 * N_COLS);
			round(inv_rnd,  b1, b0, kp -  5 * N_COLS);
			round(inv_rnd,  b0, b1, kp -  6 * N_COLS);
			round(inv_rnd,  b1, b0, kp -  7 * N_COLS);
			round(inv_rnd,  b0, b1, kp -  8 * N_COLS);
			round(inv_rnd,  b1, b0, kp -  9 * N_COLS);
#if defined(LAST_DEC_ROUND_CACHE_TABLES)
			itables(t_il);
#endif	
			round(inv_lrnd, b0, b1, kp - 10 * N_COLS);
		}

#else
	
		{   aes_32t    rnd;
#if (DEC_UNROLL == PARTIAL)
			for(rnd = 0; rnd < (cx->rn >> 1) - 1; ++rnd)
			{
				kp -= N_COLS;
				round(inv_rnd, b1, b0, kp);
				kp -= N_COLS;
				round(inv_rnd, b0, b1, kp);
			}
			kp -= N_COLS;
			round(inv_rnd, b1, b0, kp);
#else
			for(rnd = 0; rnd < cx->rn - 1; ++rnd)
			{
				kp -= N_COLS;
				round(inv_rnd, b1, b0, kp);
				l_copy(b0, b1);
			}
#endif
#if defined(LAST_DEC_ROUND_CACHE_TABLES)
			itables(t_il);
#endif	
			kp -= N_COLS;
			round(inv_lrnd, b0, b1, kp);
		}
#endif

		#ifdef	_APPLE_COMMON_CRYPTO_
		if(cbcEnable) {
			if (num_blk == 1)
			{
				// We are doing the first block so we need the IV rather than the previous
				// block for CBC (there is no previous block)
				if(in_iv) {
					state_in(b1, in_iv);
				}
				else {
					state_in(b1, cx->chainBuf);
				}
			}
			else
			{
				in -= AES_BLOCK_SIZE;
				state_in(b1, in);
			}

			// Do the CBC with b1 which is either the IV or the ciphertext of 
			// the previous block.
			cbc(b0, b1);
		}
		else {
			if (num_blk != 1) {
				/* skip this the last time through */
				in -= AES_BLOCK_SIZE;
				state_in(b1, in);
			}
		}
		#endif	/* _APPLE_COMMON_CRYPTO_ */
		state_out(out, b0);
	}

	#ifdef	_APPLE_COMMON_CRYPTO_
	/* save last ciphertext block for next op's chain */
	if(cbcEnable) {
		memmove(cx->chainBuf, lastIv, AES_BLOCK_SIZE);
	}
	#endif
	
#if defined( AES_ERR_CHK )
    return aes_good;
#endif
}

#endif

#if defined(__cplusplus)
}
#endif

#endif	// defined UseGladmanAES
