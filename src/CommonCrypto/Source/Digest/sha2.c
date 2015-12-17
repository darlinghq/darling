/* 
 * Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.
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
 ---------------------------------------------------------------------------
 Copyright (c) 2002, Dr Brian Gladman, Worcester, UK.   All rights reserved.

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
 Issue Date: 26/08/2003

 This is a byte oriented version of SHA2 that operates on arrays of bytes
 stored in memory. This code implements sha256, sha384 and sha512 but the
 latter two functions rely on efficient 64-bit integer operations that
 may not be very efficient on 32-bit machines

 The sha256 functions use a type 'sha256_ctx' to hold details of the
 current hash state and uses the following three calls:

       void sha256_begin(sha256_ctx ctx[1])
       void sha256_hash(const unsigned char data[],
                            unsigned long len, sha256_ctx ctx[1])
       void sha_end1(unsigned char hval[], sha256_ctx ctx[1])

 The first subroutine initialises a hash computation by setting up the
 context in the sha256_ctx context. The second subroutine hashes 8-bit
 bytes from array data[] into the hash state withinh sha256_ctx context,
 the number of bytes to be hashed being given by the the unsigned long
 integer len.  The third subroutine completes the hash calculation and
 places the resulting digest value in the array of 8-bit bytes hval[].

 The sha384 and sha512 functions are similar and use the interfaces:

       void sha384_begin(sha384_ctx ctx[1]);
       void sha384_hash(const unsigned char data[],
                            unsigned long len, sha384_ctx ctx[1]);
       void sha384_end(unsigned char hval[], sha384_ctx ctx[1]);

       void sha512_begin(sha512_ctx ctx[1]);
       void sha512_hash(const unsigned char data[],
                            unsigned long len, sha512_ctx ctx[1]);
       void sha512_end(unsigned char hval[], sha512_ctx ctx[1]);

 In addition there is a function sha2 that can be used to call all these
 functions using a call with a hash length parameter as follows:

       int sha2_begin(unsigned long len, sha2_ctx ctx[1]);
       void sha2_hash(const unsigned char data[],
                            unsigned long len, sha2_ctx ctx[1]);
       void sha2_end(unsigned char hval[], sha2_ctx ctx[1]);

 My thanks to Erik Andersen <andersen@codepoet.org> for testing this code
 on big-endian systems and for his assistance with corrections
*/

/*
 * Apple: Measurements indicate that we get both smaller code size and faster
 * performance when compiling this file with -O1 than with either -O3 or -Os.
 *
 * -O1
 * sha2.o 18652 bytes of text
 * 7.509 seconds to digest 100000000 bytes with SHA512
 *
 * -Os
 * sha2.o 19552 bytes of text
 * 8.693 seconds to process 100000000 bytes
 * 
 * -O3
 * sha2.o 20452 bytes of text
 * 8.535 seconds to process 100000000 bytes
 *
 * #defining UNROOL_SHA2 leads to no noticable improvement. 
 */
#include "sha2Priv.h"   /* Apple Common Digest version */

/* define the hash functions that you need          */

#ifndef _APPLE_COMMON_CRYPTO_
#define SHA_2           /* for dynamic hash length  */
#endif  /* _APPLE_COMMON_CRYPTO_ */
#define SHA_224
#define SHA_256
#define SHA_384
#define SHA_512

#if 0
#define UNROLL_SHA2     /* for SHA2 loop unroll     */
#endif

#include <string.h>     /* for memcpy() etc.        */
#include <stdlib.h>     /* for _lrotr with VC++     */

/* #include "sha2.h" */

#if defined(__cplusplus)
extern "C"
{
#endif

/*  PLATFORM SPECIFIC INCLUDES AND BYTE ORDER IN 32-BIT WORDS

    To obtain the highest speed on processors with 32-bit words, this code
    needs to determine the byte order of the target machine. The following
    block of code is an attempt to capture the most obvious ways in which
    various environemnts define byte order. It may well fail, in which case
    the definitions will need to be set by editing at the points marked
    **** EDIT HERE IF NECESSARY **** below.  My thanks go to Peter Gutmann
    for his assistance with this endian detection nightmare.
*/

#define BRG_LITTLE_ENDIAN   1234 /* byte 0 is least significant (i386) */
#define BRG_BIG_ENDIAN      4321 /* byte 0 is most significant (mc68k) */

#if defined(__GNUC__) || defined(__GNU_LIBRARY__)
#  if defined(__FreeBSD__) || defined(__OpenBSD__)
#    include <sys/endian.h>
#  elif defined( BSD ) && ( BSD >= 199103 )
#      include <machine/endian.h>
#  elif defined(__APPLE__)
#    if defined(__BIG_ENDIAN__) && !defined( BIG_ENDIAN )
#      define BIG_ENDIAN
#    elif defined(__LITTLE_ENDIAN__) && !defined( LITTLE_ENDIAN )
#      define LITTLE_ENDIAN
#    endif
#  else
#    include <endian.h>
#    if !defined(__BEOS__)
#      include <byteswap.h>
#    endif
#  endif
#endif

#if !defined(PLATFORM_BYTE_ORDER)
#  if defined(LITTLE_ENDIAN) || defined(BIG_ENDIAN)
#    if    defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif !defined(LITTLE_ENDIAN) &&  defined(BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    elif defined(BYTE_ORDER) && (BYTE_ORDER == LITTLE_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif defined(BYTE_ORDER) && (BYTE_ORDER == BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    endif
#  elif defined(_LITTLE_ENDIAN) || defined(_BIG_ENDIAN)
#    if    defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif !defined(_LITTLE_ENDIAN) &&  defined(_BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    elif defined(_BYTE_ORDER) && (_BYTE_ORDER == _LITTLE_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif defined(_BYTE_ORDER) && (_BYTE_ORDER == _BIG_ENDIAN)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#   endif
#  elif defined(__LITTLE_ENDIAN__) || defined(__BIG_ENDIAN__)
#    if    defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif !defined(__LITTLE_ENDIAN__) &&  defined(__BIG_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __LITTLE_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#    elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __BIG_ENDIAN__)
#      define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#    endif
#  endif
#endif

/*  if the platform is still unknown, try to find its byte order    */
/*  from commonly used machine defines                              */

#if !defined(PLATFORM_BYTE_ORDER)

#if   defined( __alpha__ ) || defined( __alpha ) || defined( i386 )       || \
      defined( __i386__ )  || defined( _M_I86 )  || defined( _M_IX86 )    || \
      defined( __OS2__ )   || defined( sun386 )  || defined( __TURBOC__ ) || \
      defined( vax )       || defined( vms )     || defined( VMS )        || \
      defined( __VMS )     || defined(__x86_64__)
#  define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN

#elif defined( AMIGA )    || defined( applec )  || defined( __AS400__ )  || \
      defined( _CRAY )    || defined( __hppa )  || defined( __hp9000 )   || \
      defined( ibm370 )   || defined( mc68000 ) || defined( m68k )       || \
      defined( __MRC__ )  || defined( __MVS__ ) || defined( __MWERKS__ ) || \
      defined( sparc )    || defined( __sparc)  || defined( SYMANTEC_C ) || \
      defined( __TANDEM ) || defined( THINK_C ) || defined( __VMCMS__ )
#  define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN

#elif 0     /* **** EDIT HERE IF NECESSARY **** */
#  define PLATFORM_BYTE_ORDER BRG_LITTLE_ENDIAN
#elif 0     /* **** EDIT HERE IF NECESSARY **** */
#  define PLATFORM_BYTE_ORDER BRG_BIG_ENDIAN
#else
#  error Please edit sha2.c (line 184 or 186) to set the platform byte order
#endif

#endif

#ifdef _MSC_VER
#pragma intrinsic(memcpy)
#endif

#if 0 && defined(_MSC_VER)
#define rotl32 _lrotl
#define rotr32 _lrotr
#else
#define rotl32(x,n)   (((x) << n) | ((x) >> (32 - n)))
#define rotr32(x,n)   (((x) >> n) | ((x) << (32 - n)))
#endif

#if !defined(bswap_32)
#define bswap_32(x) ((rotr32((x), 24) & 0x00ff00ff) | (rotr32((x), 8) & 0xff00ff00))
#endif

#if (PLATFORM_BYTE_ORDER == BRG_LITTLE_ENDIAN)
#define SWAP_BYTES
#else
#undef  SWAP_BYTES
#endif

#if 0

#define ch(x,y,z)       (((x) & (y)) ^ (~(x) & (z)))
#define maj(x,y,z)      (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#else   /* Thanks to Rich Schroeppel and Colin Plumb for the following      */

#define ch(x,y,z)       ((z) ^ ((x) & ((y) ^ (z))))
#define maj(x,y,z)      (((x) & (y)) | ((z) & ((x) ^ (y))))

#endif

/* round transforms for SHA256 and SHA512 compression functions */

#define vf(n,i) v[(n - i) & 7]

#define hf(i) (p[i & 15] += \
    g_1(p[(i + 14) & 15]) + p[(i + 9) & 15] + g_0(p[(i + 1) & 15]))

#define v_cycle(i,j)                                \
    vf(7,i) += (j ? hf(i) : p[i]) + k_0[i+j]        \
    + s_1(vf(4,i)) + ch(vf(4,i),vf(5,i),vf(6,i));   \
    vf(3,i) += vf(7,i);                             \
    vf(7,i) += s_0(vf(0,i))+ maj(vf(0,i),vf(1,i),vf(2,i))

#if defined(SHA_224) || defined(SHA_256)

#define SHA256_MASK (SHA256_BLOCK_SIZE - 1)

#if defined(SWAP_BYTES)
#define bsw_32(p,n) \
    { int _i = (n); while(_i--) ((sha2_32t*)p)[_i] = bswap_32(((sha2_32t*)p)[_i]); }
#else
#define bsw_32(p,n)
#endif

#define s_0(x)  (rotr32((x),  2) ^ rotr32((x), 13) ^ rotr32((x), 22))
#define s_1(x)  (rotr32((x),  6) ^ rotr32((x), 11) ^ rotr32((x), 25))
#define g_0(x)  (rotr32((x),  7) ^ rotr32((x), 18) ^ ((x) >>  3))
#define g_1(x)  (rotr32((x), 17) ^ rotr32((x), 19) ^ ((x) >> 10))
#define k_0     k256

/* rotated SHA256 round definition. Rather than swapping variables as in    */
/* FIPS-180, different variables are 'rotated' on each round, returning     */
/* to their starting positions every eight rounds                           */

#define q(n)  v##n

#define one_cycle(a,b,c,d,e,f,g,h,k,w)  \
    q(h) += s_1(q(e)) + ch(q(e), q(f), q(g)) + k + w; \
    q(d) += q(h); q(h) += s_0(q(a)) + maj(q(a), q(b), q(c))

/* SHA256 mixing data   */
#if defined (SHA256_USE_ASSEMBLY) && (defined(__x86_64__) || defined(__i386__))
	const sha2_32t K256[64] =
#else
static const sha2_32t k256[64] =
#endif
{   0x428a2f98ul, 0x71374491ul, 0xb5c0fbcful, 0xe9b5dba5ul,
    0x3956c25bul, 0x59f111f1ul, 0x923f82a4ul, 0xab1c5ed5ul,
    0xd807aa98ul, 0x12835b01ul, 0x243185beul, 0x550c7dc3ul,
    0x72be5d74ul, 0x80deb1feul, 0x9bdc06a7ul, 0xc19bf174ul,
    0xe49b69c1ul, 0xefbe4786ul, 0x0fc19dc6ul, 0x240ca1ccul,
    0x2de92c6ful, 0x4a7484aaul, 0x5cb0a9dcul, 0x76f988daul,
    0x983e5152ul, 0xa831c66dul, 0xb00327c8ul, 0xbf597fc7ul,
    0xc6e00bf3ul, 0xd5a79147ul, 0x06ca6351ul, 0x14292967ul,
    0x27b70a85ul, 0x2e1b2138ul, 0x4d2c6dfcul, 0x53380d13ul,
    0x650a7354ul, 0x766a0abbul, 0x81c2c92eul, 0x92722c85ul,
    0xa2bfe8a1ul, 0xa81a664bul, 0xc24b8b70ul, 0xc76c51a3ul,
    0xd192e819ul, 0xd6990624ul, 0xf40e3585ul, 0x106aa070ul,
    0x19a4c116ul, 0x1e376c08ul, 0x2748774cul, 0x34b0bcb5ul,
    0x391c0cb3ul, 0x4ed8aa4aul, 0x5b9cca4ful, 0x682e6ff3ul,
    0x748f82eeul, 0x78a5636ful, 0x84c87814ul, 0x8cc70208ul,
    0x90befffaul, 0xa4506cebul, 0xbef9a3f7ul, 0xc67178f2ul,
};

/* Compile 64 bytes of hash data into SHA256 digest value   */
/* NOTE: this routine assumes that the byte order in the    */
/* ctx->wbuf[] at this point is such that low address bytes */
/* in the ORIGINAL byte stream will go into the high end of */
/* words on BOTH big and little endian systems              */

#if defined (SHA256_USE_ASSEMBLY) && (defined(__x86_64__) || defined(__i386__))
extern	sha2_void sha256_compile(sha256_ctx ctx[1]);
#else
static sha2_void sha256_compile(sha256_ctx ctx[1])
{
#if !defined(UNROLL_SHA2)

    sha2_32t j, *p = ctx->wbuf, v[8];

    memcpy(v, ctx->hash, 8 * sizeof(sha2_32t));

    for(j = 0; j < 64; j += 16)
    {
        v_cycle( 0, j); v_cycle( 1, j);
        v_cycle( 2, j); v_cycle( 3, j);
        v_cycle( 4, j); v_cycle( 5, j);
        v_cycle( 6, j); v_cycle( 7, j);
        v_cycle( 8, j); v_cycle( 9, j);
        v_cycle(10, j); v_cycle(11, j);
        v_cycle(12, j); v_cycle(13, j);
        v_cycle(14, j); v_cycle(15, j);
    }

    ctx->hash[0] += v[0]; ctx->hash[1] += v[1];
    ctx->hash[2] += v[2]; ctx->hash[3] += v[3];
    ctx->hash[4] += v[4]; ctx->hash[5] += v[5];
    ctx->hash[6] += v[6]; ctx->hash[7] += v[7];

#else

    sha2_32t *p = ctx->wbuf,v0,v1,v2,v3,v4,v5,v6,v7;

    v0 = ctx->hash[0]; v1 = ctx->hash[1];
    v2 = ctx->hash[2]; v3 = ctx->hash[3];
    v4 = ctx->hash[4]; v5 = ctx->hash[5];
    v6 = ctx->hash[6]; v7 = ctx->hash[7];

    one_cycle(0,1,2,3,4,5,6,7,k256[ 0],p[ 0]);
    one_cycle(7,0,1,2,3,4,5,6,k256[ 1],p[ 1]);
    one_cycle(6,7,0,1,2,3,4,5,k256[ 2],p[ 2]);
    one_cycle(5,6,7,0,1,2,3,4,k256[ 3],p[ 3]);
    one_cycle(4,5,6,7,0,1,2,3,k256[ 4],p[ 4]);
    one_cycle(3,4,5,6,7,0,1,2,k256[ 5],p[ 5]);
    one_cycle(2,3,4,5,6,7,0,1,k256[ 6],p[ 6]);
    one_cycle(1,2,3,4,5,6,7,0,k256[ 7],p[ 7]);
    one_cycle(0,1,2,3,4,5,6,7,k256[ 8],p[ 8]);
    one_cycle(7,0,1,2,3,4,5,6,k256[ 9],p[ 9]);
    one_cycle(6,7,0,1,2,3,4,5,k256[10],p[10]);
    one_cycle(5,6,7,0,1,2,3,4,k256[11],p[11]);
    one_cycle(4,5,6,7,0,1,2,3,k256[12],p[12]);
    one_cycle(3,4,5,6,7,0,1,2,k256[13],p[13]);
    one_cycle(2,3,4,5,6,7,0,1,k256[14],p[14]);
    one_cycle(1,2,3,4,5,6,7,0,k256[15],p[15]);

    one_cycle(0,1,2,3,4,5,6,7,k256[16],hf( 0));
    one_cycle(7,0,1,2,3,4,5,6,k256[17],hf( 1));
    one_cycle(6,7,0,1,2,3,4,5,k256[18],hf( 2));
    one_cycle(5,6,7,0,1,2,3,4,k256[19],hf( 3));
    one_cycle(4,5,6,7,0,1,2,3,k256[20],hf( 4));
    one_cycle(3,4,5,6,7,0,1,2,k256[21],hf( 5));
    one_cycle(2,3,4,5,6,7,0,1,k256[22],hf( 6));
    one_cycle(1,2,3,4,5,6,7,0,k256[23],hf( 7));
    one_cycle(0,1,2,3,4,5,6,7,k256[24],hf( 8));
    one_cycle(7,0,1,2,3,4,5,6,k256[25],hf( 9));
    one_cycle(6,7,0,1,2,3,4,5,k256[26],hf(10));
    one_cycle(5,6,7,0,1,2,3,4,k256[27],hf(11));
    one_cycle(4,5,6,7,0,1,2,3,k256[28],hf(12));
    one_cycle(3,4,5,6,7,0,1,2,k256[29],hf(13));
    one_cycle(2,3,4,5,6,7,0,1,k256[30],hf(14));
    one_cycle(1,2,3,4,5,6,7,0,k256[31],hf(15));

    one_cycle(0,1,2,3,4,5,6,7,k256[32],hf( 0));
    one_cycle(7,0,1,2,3,4,5,6,k256[33],hf( 1));
    one_cycle(6,7,0,1,2,3,4,5,k256[34],hf( 2));
    one_cycle(5,6,7,0,1,2,3,4,k256[35],hf( 3));
    one_cycle(4,5,6,7,0,1,2,3,k256[36],hf( 4));
    one_cycle(3,4,5,6,7,0,1,2,k256[37],hf( 5));
    one_cycle(2,3,4,5,6,7,0,1,k256[38],hf( 6));
    one_cycle(1,2,3,4,5,6,7,0,k256[39],hf( 7));
    one_cycle(0,1,2,3,4,5,6,7,k256[40],hf( 8));
    one_cycle(7,0,1,2,3,4,5,6,k256[41],hf( 9));
    one_cycle(6,7,0,1,2,3,4,5,k256[42],hf(10));
    one_cycle(5,6,7,0,1,2,3,4,k256[43],hf(11));
    one_cycle(4,5,6,7,0,1,2,3,k256[44],hf(12));
    one_cycle(3,4,5,6,7,0,1,2,k256[45],hf(13));
    one_cycle(2,3,4,5,6,7,0,1,k256[46],hf(14));
    one_cycle(1,2,3,4,5,6,7,0,k256[47],hf(15));

    one_cycle(0,1,2,3,4,5,6,7,k256[48],hf( 0));
    one_cycle(7,0,1,2,3,4,5,6,k256[49],hf( 1));
    one_cycle(6,7,0,1,2,3,4,5,k256[50],hf( 2));
    one_cycle(5,6,7,0,1,2,3,4,k256[51],hf( 3));
    one_cycle(4,5,6,7,0,1,2,3,k256[52],hf( 4));
    one_cycle(3,4,5,6,7,0,1,2,k256[53],hf( 5));
    one_cycle(2,3,4,5,6,7,0,1,k256[54],hf( 6));
    one_cycle(1,2,3,4,5,6,7,0,k256[55],hf( 7));
    one_cycle(0,1,2,3,4,5,6,7,k256[56],hf( 8));
    one_cycle(7,0,1,2,3,4,5,6,k256[57],hf( 9));
    one_cycle(6,7,0,1,2,3,4,5,k256[58],hf(10));
    one_cycle(5,6,7,0,1,2,3,4,k256[59],hf(11));
    one_cycle(4,5,6,7,0,1,2,3,k256[60],hf(12));
    one_cycle(3,4,5,6,7,0,1,2,k256[61],hf(13));
    one_cycle(2,3,4,5,6,7,0,1,k256[62],hf(14));
    one_cycle(1,2,3,4,5,6,7,0,k256[63],hf(15));

    ctx->hash[0] += v0; ctx->hash[1] += v1;
    ctx->hash[2] += v2; ctx->hash[3] += v3;
    ctx->hash[4] += v4; ctx->hash[5] += v5;
    ctx->hash[6] += v6; ctx->hash[7] += v7;
#endif
}
#endif	//	SHA256_USE_ASSEMBLY 

/* SHA256 hash data in an array of bytes into hash buffer   */
/* and call the hash_compile function as required.          */
#ifdef		_APPLE_COMMON_CRYPTO_
int CC_SHA256_Update(CC_SHA256_CTX *ctx, const void *data, CC_LONG len)
#else
sha2_void sha256_hash(const unsigned char data[], unsigned long len, sha256_ctx ctx[1])
#endif  /* _APPLE_COMMON_CRYPTO_ */
{   sha2_32t pos = (sha2_32t)(ctx->count[0] & SHA256_MASK),
             space = SHA256_BLOCK_SIZE - pos;
    const unsigned char *sp = data;

    if((ctx->count[0] += len) < len)
        ++(ctx->count[1]);

    while(len >= space)     /* tranfer whole blocks while possible  */
    {
        memcpy(((unsigned char*)ctx->wbuf) + pos, sp, space);
        sp += space; len -= space; space = SHA256_BLOCK_SIZE; pos = 0;
        bsw_32(ctx->wbuf, SHA256_BLOCK_SIZE >> 2)
        sha256_compile(ctx);
    }

    memcpy(((unsigned char*)ctx->wbuf) + pos, sp, len);
	return 1;
}

/* SHA256 Final padding and digest calculation  */

static sha2_void sha_end1(unsigned char hval[], sha256_ctx ctx[1], const unsigned int hlen)
{   sha2_32t    i = (sha2_32t)(ctx->count[0] & SHA256_MASK);

    /* put bytes in the buffer in an order in which references to   */
    /* 32-bit words will put bytes with lower addresses into the    */
    /* top of 32 bit words on BOTH big and little endian machines   */
    bsw_32(ctx->wbuf, (i + 3) >> 2)

    /* we now need to mask valid bytes and add the padding which is */
    /* a single 1 bit and as many zero bits as necessary. Note that */
    /* we can always add the first padding byte here because the    */
    /* buffer always has at least one empty slot                    */
    ctx->wbuf[i >> 2] &= 0xffffff80 << 8 * (~i & 3);
    ctx->wbuf[i >> 2] |= 0x00000080 << 8 * (~i & 3);

    /* we need 9 or more empty positions, one for the padding byte  */
    /* (above) and eight for the length count.  If there is not     */
    /* enough space pad and empty the buffer                        */
    if(i > SHA256_BLOCK_SIZE - 9)
    {
        if(i < 60) ctx->wbuf[15] = 0;
        sha256_compile(ctx);
        i = 0;
    }
    else    /* compute a word index for the empty buffer positions  */
        i = (i >> 2) + 1;

    while(i < 14) /* and zero pad all but last two positions        */
        ctx->wbuf[i++] = 0;

    /* the following 32-bit length fields are assembled in the      */
    /* wrong byte order on little endian machines but this is       */
    /* corrected later since they are only ever used as 32-bit      */
    /* word values.                                                 */
    ctx->wbuf[14] = (ctx->count[1] << 3) | (ctx->count[0] >> 29);
    ctx->wbuf[15] = ctx->count[0] << 3;
    sha256_compile(ctx);

    /* extract the hash value as bytes in case the hash buffer is   */
    /* mislaigned for 32-bit words                                  */
    for(i = 0; i < hlen; ++i)
        hval[i] = (unsigned char)(ctx->hash[i >> 2] >> (8 * (~i & 3)));
}

#endif

#if defined(SHA_224)

const sha2_32t i224[8] =
{
    0xc1059ed8ul, 0x367cd507ul, 0x3070dd17ul, 0xf70e5939ul,
    0xffc00b31ul, 0x68581511ul, 0x64f98fa7ul, 0xbefa4fa4ul
};

#ifdef _APPLE_COMMON_CRYPTO_
int sha224_begin(sha256_ctx ctx[1])
#else
sha2_void sha224_begin(sha256_ctx ctx[1])
#endif
{
    ctx->count[0] = ctx->count[1] = 0;
    memcpy(ctx->hash, i224, 8 * sizeof(sha2_32t));
	return 1;
}

#ifdef _APPLE_COMMON_CRYPTO_
int sha224_end(unsigned char hval[], sha256_ctx ctx[1])
#else
sha2_void sha224_end(unsigned char hval[], sha256_ctx ctx[1])
#endif
{
    sha_end1(hval, ctx, SHA224_DIGEST_SIZE);
	return 1;
}

#ifndef _APPLE_COMMON_CRYPTO_
sha2_void sha224(unsigned char hval[], const unsigned char data[], unsigned long len)
{   sha224_ctx  cx[1];

    sha224_begin(cx);
    sha224_hash(data, len, cx);
    sha_end1(hval, cx, SHA224_DIGEST_SIZE);
}
#endif	/* _APPLE_COMMON_CRYPTO_ */

/* provide an actual entry for this instead of #defining it */
extern int CC_SHA224_Update(CC_SHA256_CTX *c, const void *data, CC_LONG len)
{
	return CC_SHA256_Update(c, data, len);
}

#endif	/* SHA_224 */

#if defined(SHA_256)

static const sha2_32t i256[8] =
{
    0x6a09e667ul, 0xbb67ae85ul, 0x3c6ef372ul, 0xa54ff53aul,
    0x510e527ful, 0x9b05688cul, 0x1f83d9abul, 0x5be0cd19ul
};

int sha256_begin(sha256_ctx ctx[1])
{
    ctx->count[0] = ctx->count[1] = 0;
    memcpy(ctx->hash, i256, 8 * sizeof(sha2_32t));
	return 1;
}

int sha256_end(unsigned char hval[], sha256_ctx ctx[1])
{
    sha_end1(hval, ctx, SHA256_DIGEST_SIZE);
	return 1;
}

#ifndef _APPLE_COMMON_CRYPTO_
sha2_void sha256(unsigned char hval[], const unsigned char data[], unsigned long len)
{   sha256_ctx  cx[1];

    sha256_begin(cx);
    sha256_hash(data, len, cx);
    sha_end1(hval, cx, SHA256_DIGEST_SIZE);
}
#endif  /* _APPLE_COMMON_CRYPTO_ */

#endif

#if defined(SHA_384) || defined(SHA_512)

#define SHA512_MASK (SHA512_BLOCK_SIZE - 1)

#define rotr64(x,n)   (((x) >> n) | ((x) << (64 - n)))

#if !defined(bswap_64)
#define bswap_64(x) (((sha2_64t)(bswap_32((sha2_32t)(x)))) << 32 | bswap_32((sha2_32t)((x) >> 32)))
#endif

#if defined(SWAP_BYTES)
#define bsw_64(p,n) \
    { int _i = (n); while(_i--) ((sha2_64t*)p)[_i] = bswap_64(((sha2_64t*)p)[_i]); }
#else
#define bsw_64(p,n)
#endif

/* SHA512 mixing function definitions   */

#ifdef   s_0
# undef  s_0
# undef  s_1
# undef  g_0
# undef  g_1
# undef  k_0
#endif

#define s_0(x)  (rotr64((x), 28) ^ rotr64((x), 34) ^ rotr64((x), 39))
#define s_1(x)  (rotr64((x), 14) ^ rotr64((x), 18) ^ rotr64((x), 41))
#define g_0(x)  (rotr64((x),  1) ^ rotr64((x),  8) ^ ((x) >>  7))
#define g_1(x)  (rotr64((x), 19) ^ rotr64((x), 61) ^ ((x) >>  6))
#define k_0     k512

/* SHA384/SHA512 mixing data    */

static const sha2_64t  k512[80] =
{
    0x428a2f98d728ae22ull, 0x7137449123ef65cdull,
    0xb5c0fbcfec4d3b2full, 0xe9b5dba58189dbbcull,
    0x3956c25bf348b538ull, 0x59f111f1b605d019ull,
    0x923f82a4af194f9bull, 0xab1c5ed5da6d8118ull,
    0xd807aa98a3030242ull, 0x12835b0145706fbeull,
    0x243185be4ee4b28cull, 0x550c7dc3d5ffb4e2ull,
    0x72be5d74f27b896full, 0x80deb1fe3b1696b1ull,
    0x9bdc06a725c71235ull, 0xc19bf174cf692694ull,
    0xe49b69c19ef14ad2ull, 0xefbe4786384f25e3ull,
    0x0fc19dc68b8cd5b5ull, 0x240ca1cc77ac9c65ull,
    0x2de92c6f592b0275ull, 0x4a7484aa6ea6e483ull,
    0x5cb0a9dcbd41fbd4ull, 0x76f988da831153b5ull,
    0x983e5152ee66dfabull, 0xa831c66d2db43210ull,
    0xb00327c898fb213full, 0xbf597fc7beef0ee4ull,
    0xc6e00bf33da88fc2ull, 0xd5a79147930aa725ull,
    0x06ca6351e003826full, 0x142929670a0e6e70ull,
    0x27b70a8546d22ffcull, 0x2e1b21385c26c926ull,
    0x4d2c6dfc5ac42aedull, 0x53380d139d95b3dfull,
    0x650a73548baf63deull, 0x766a0abb3c77b2a8ull,
    0x81c2c92e47edaee6ull, 0x92722c851482353bull,
    0xa2bfe8a14cf10364ull, 0xa81a664bbc423001ull,
    0xc24b8b70d0f89791ull, 0xc76c51a30654be30ull,
    0xd192e819d6ef5218ull, 0xd69906245565a910ull,
    0xf40e35855771202aull, 0x106aa07032bbd1b8ull,
    0x19a4c116b8d2d0c8ull, 0x1e376c085141ab53ull,
    0x2748774cdf8eeb99ull, 0x34b0bcb5e19b48a8ull,
    0x391c0cb3c5c95a63ull, 0x4ed8aa4ae3418acbull,
    0x5b9cca4f7763e373ull, 0x682e6ff3d6b2b8a3ull,
    0x748f82ee5defb2fcull, 0x78a5636f43172f60ull,
    0x84c87814a1f0ab72ull, 0x8cc702081a6439ecull,
    0x90befffa23631e28ull, 0xa4506cebde82bde9ull,
    0xbef9a3f7b2c67915ull, 0xc67178f2e372532bull,
    0xca273eceea26619cull, 0xd186b8c721c0c207ull,
    0xeada7dd6cde0eb1eull, 0xf57d4f7fee6ed178ull,
    0x06f067aa72176fbaull, 0x0a637dc5a2c898a6ull,
    0x113f9804bef90daeull, 0x1b710b35131c471bull,
    0x28db77f523047d84ull, 0x32caab7b40c72493ull,
    0x3c9ebe0a15c9bebcull, 0x431d67c49c100d4cull,
    0x4cc5d4becb3e42b6ull, 0x597f299cfc657e2aull,
    0x5fcb6fab3ad6faecull, 0x6c44198c4a475817ull
};

/* Compile 128 bytes of hash data into SHA384/512 digest    */
/* NOTE: this routine assumes that the byte order in the    */
/* ctx->wbuf[] at this point is such that low address bytes */
/* in the ORIGINAL byte stream will go into the high end of */
/* words on BOTH big and little endian systems              */

static sha2_void sha512_compile(sha512_ctx ctx[1])
{   sha2_64t    v[8], *p = ctx->wbuf;
    sha2_32t    j;

    memcpy(v, ctx->hash, 8 * sizeof(sha2_64t));

    for(j = 0; j < 80; j += 16)
    {
        v_cycle( 0, j); v_cycle( 1, j);
        v_cycle( 2, j); v_cycle( 3, j);
        v_cycle( 4, j); v_cycle( 5, j);
        v_cycle( 6, j); v_cycle( 7, j);
        v_cycle( 8, j); v_cycle( 9, j);
        v_cycle(10, j); v_cycle(11, j);
        v_cycle(12, j); v_cycle(13, j);
        v_cycle(14, j); v_cycle(15, j);
    }

    ctx->hash[0] += v[0]; ctx->hash[1] += v[1];
    ctx->hash[2] += v[2]; ctx->hash[3] += v[3];
    ctx->hash[4] += v[4]; ctx->hash[5] += v[5];
    ctx->hash[6] += v[6]; ctx->hash[7] += v[7];
}

/* Compile 128 bytes of hash data into SHA256 digest value  */
/* NOTE: this routine assumes that the byte order in the    */
/* ctx->wbuf[] at this point is in such an order that low   */
/* address bytes in the ORIGINAL byte stream placed in this */
/* buffer will now go to the high end of words on BOTH big  */
/* and little endian systems                                */

#ifdef  _APPLE_COMMON_CRYPTO_
int CC_SHA512_Update(CC_SHA512_CTX *ctx, const void *data, CC_LONG len)
#else
sha2_void sha512_hash(const unsigned char data[], unsigned long len, sha512_ctx ctx[1])
#endif
{   sha2_32t pos = (sha2_32t)(ctx->count[0] & SHA512_MASK),
             space = SHA512_BLOCK_SIZE - pos;
    const unsigned char *sp = data;

    if((ctx->count[0] += len) < len)
        ++(ctx->count[1]);

    while(len >= space)     /* tranfer whole blocks while possible  */
    {
        memcpy(((unsigned char*)ctx->wbuf) + pos, sp, space);
        sp += space; len -= space; space = SHA512_BLOCK_SIZE; pos = 0;
        bsw_64(ctx->wbuf, SHA512_BLOCK_SIZE >> 3);
        sha512_compile(ctx);
    }

    memcpy(((unsigned char*)ctx->wbuf) + pos, sp, len);
	return 1;
}

/* SHA384/512 Final padding and digest calculation  */

static void sha_end2(unsigned char hval[], sha512_ctx ctx[1], const unsigned int hlen)
{   sha2_32t    i = (sha2_32t)(ctx->count[0] & SHA512_MASK);

    /* put bytes in the buffer in an order in which references to   */
    /* 32-bit words will put bytes with lower addresses into the    */
    /* top of 32 bit words on BOTH big and little endian machines   */
    bsw_64(ctx->wbuf, (i + 7) >> 3);

    /* we now need to mask valid bytes and add the padding which is */
    /* a single 1 bit and as many zero bits as necessary. Note that */
    /* we can always add the first padding byte here because the    */
    /* buffer always has at least one empty slot                    */
    ctx->wbuf[i >> 3] &= 0xffffffffffffff00ull << 8 * (~i & 7);
    ctx->wbuf[i >> 3] |= 0x0000000000000080ull << 8 * (~i & 7);

    /* we need 17 or more empty byte positions, one for the padding */
    /* byte (above) and sixteen for the length count.  If there is  */
    /* not enough space pad and empty the buffer                    */
    if(i > SHA512_BLOCK_SIZE - 17)
    {
        if(i < 120) ctx->wbuf[15] = 0;
        sha512_compile(ctx);
        i = 0;
    }
    else
        i = (i >> 3) + 1;

    while(i < 14)
        ctx->wbuf[i++] = 0;

    /* the following 64-bit length fields are assembled in the      */
    /* wrong byte order on little endian machines but this is       */
    /* corrected later since they are only ever used as 64-bit      */
    /* word values.                                                 */
    ctx->wbuf[14] = (ctx->count[1] << 3) | (ctx->count[0] >> 61);
    ctx->wbuf[15] = ctx->count[0] << 3;
    sha512_compile(ctx);

    /* extract the hash value as bytes in case the hash buffer is   */
    /* misaligned for 32-bit words                                  */
    for(i = 0; i < hlen; ++i)
        hval[i] = (unsigned char)(ctx->hash[i >> 3] >> (8 * (~i & 7)));
}

#endif

#if defined(SHA_384)

/* SHA384 initialisation data   */

static const sha2_64t  i384[80] =
{
    0xcbbb9d5dc1059ed8ull, 0x629a292a367cd507ull,
    0x9159015a3070dd17ull, 0x152fecd8f70e5939ull,
    0x67332667ffc00b31ull, 0x8eb44a8768581511ull,
    0xdb0c2e0d64f98fa7ull, 0x47b5481dbefa4fa4ull
};

#ifdef _APPLE_COMMON_CRYPTO_
int sha384_begin(sha384_ctx *ctx)
#else
sha2_void sha384_begin(sha384_ctx ctx[1])
#endif
{
    ctx->count[0] = ctx->count[1] = 0;
    memcpy(ctx->hash, i384, 8 * sizeof(sha2_64t));
	return 1;
}

#ifdef _APPLE_COMMON_CRYPTO_
int sha384_end(unsigned char *hval, sha384_ctx *ctx)
#else
sha2_void sha384_end(unsigned char hval[], sha384_ctx ctx[1])
#endif
{
    sha_end2(hval, ctx, SHA384_DIGEST_SIZE);
	return 1;
}

/* provide an actual entry for this instead of #defining it */
extern int CC_SHA384_Update(CC_SHA512_CTX *c, const void *data, CC_LONG len)
{
	return CC_SHA512_Update(c, data, len);
}


#ifndef _APPLE_COMMON_CRYPTO_
sha2_void sha384(unsigned char hval[], const unsigned char data[], unsigned long len)
{   sha384_ctx  cx[1];

    sha384_begin(cx);
    sha384_hash(data, len, cx);
    sha_end2(hval, cx, SHA384_DIGEST_SIZE);
}
#endif  /* _APPLE_COMMON_CRYPTO_ */
#endif

#if defined(SHA_512)

/* SHA512 initialisation data   */

static const sha2_64t  i512[80] =
{
    0x6a09e667f3bcc908ull, 0xbb67ae8584caa73bull,
    0x3c6ef372fe94f82bull, 0xa54ff53a5f1d36f1ull,
    0x510e527fade682d1ull, 0x9b05688c2b3e6c1full,
    0x1f83d9abfb41bd6bull, 0x5be0cd19137e2179ull
};

#ifdef _APPLE_COMMON_CRYPTO_
int sha512_begin(sha512_ctx *ctx)
#else
sha2_void sha512_begin(sha512_ctx ctx[1])
#endif
{
    ctx->count[0] = ctx->count[1] = 0;
    memcpy(ctx->hash, i512, 8 * sizeof(sha2_64t));
	return 1;
}

#ifdef _APPLE_COMMON_CRYPTO_
int sha512_end(unsigned char *hval, sha512_ctx *ctx)
#else
sha2_void sha512_end(unsigned char hval[], sha512_ctx ctx[1])
#endif
{
    sha_end2(hval, ctx, SHA512_DIGEST_SIZE);
	return 1;
}

#ifndef _APPLE_COMMON_CRYPTO_
sha2_void sha512(unsigned char hval[], const unsigned char data[], unsigned long len)
{   sha512_ctx  cx[1];

    sha512_begin(cx);
    sha512_hash(data, len, cx);
    sha_end2(hval, cx, SHA512_DIGEST_SIZE);
}
#endif  /* _APPLE_COMMON_CRYPTO_ */

#endif

#if defined(SHA_2)

#define CTX_224(x)  ((x)->uu->ctx256)
#define CTX_256(x)  ((x)->uu->ctx256)
#define CTX_384(x)  ((x)->uu->ctx512)
#define CTX_512(x)  ((x)->uu->ctx512)

/* SHA2 initialisation */

sha2_int sha2_begin(unsigned long len, sha2_ctx ctx[1])
{   unsigned long   l = len;
    switch(len)
    {
#if defined(SHA224)
        case 224:
        case  28:   CTX_256(ctx)->count[0] = CTX_256(ctx)->count[1] = 0;
                    memcpy(CTX_256(ctx)->hash, i224, 32);
                    ctx->sha2_len = 28; return SHA2_GOOD;
#endif
#if defined(SHA256)
        case 256:
        case  32:   CTX_256(ctx)->count[0] = CTX_256(ctx)->count[1] = 0;
                    memcpy(CTX_256(ctx)->hash, i256, 32);
                    ctx->sha2_len = 32; return SHA2_GOOD;
#endif
#if defined(SHA384)
        case 384:
        case  48:   CTX_384(ctx)->count[0] = CTX_384(ctx)->count[1] = 0;
                    memcpy(CTX_384(ctx)->hash, i384, 64);
                    ctx->sha2_len = 48; return SHA2_GOOD;
#endif
#if defined(SHA512)
        case 512:
        case  64:   CTX_512(ctx)->count[0] = CTX_512(ctx)->count[1] = 0;
                    memcpy(CTX_512(ctx)->hash, i512, 64);
                    ctx->sha2_len = 64; return SHA2_GOOD;
#endif
        default:    return SHA2_BAD;
    }
}

sha2_void sha2_hash(const unsigned char data[], unsigned long len, sha2_ctx ctx[1])
{
    switch(ctx->sha2_len)
    {
#if defined(SHA224)
        case 28: sha224_hash(data, len, CTX_224(ctx)); return;
#endif
#if defined(SHA256)
        case 32: sha256_hash(data, len, CTX_256(ctx)); return;
#endif
#if defined(SHA384)
        case 48: sha384_hash(data, len, CTX_384(ctx)); return;
#endif
#if defined(SHA512)
        case 64: sha512_hash(data, len, CTX_512(ctx)); return;
#endif
    }
}

sha2_void sha2_end(unsigned char hval[], sha2_ctx ctx[1])
{
    switch(ctx->sha2_len)
    {
#if defined(SHA224)
        case 28: sha_end1(hval, CTX_224(ctx), SHA224_DIGEST_SIZE); return;
#endif
#if defined(SHA256)
        case 32: sha_end1(hval, CTX_256(ctx), SHA256_DIGEST_SIZE); return;
#endif
#if defined(SHA384)
        case 48: sha_end2(hval, CTX_384(ctx), SHA384_DIGEST_SIZE); return;
#endif
#if defined(SHA512)
        case 64: sha_end2(hval, CTX_512(ctx), SHA512_DIGEST_SIZE); return;
#endif
    }
}

sha2_int sha2(unsigned char hval[], unsigned long size,
                                const unsigned char data[], unsigned long len)
{   sha2_ctx    cx[1];

    if(sha2_begin(size, cx) == SHA2_GOOD)
    {
        sha2_hash(data, len, cx); sha2_end(hval, cx); return SHA2_GOOD;
    }
    else
        return SHA2_BAD;
}

#endif  /* SHA2 */

#if defined(__cplusplus)
}
#endif

CC_DIGEST_ONE_SHOT(CC_SHA224, CC_SHA256_CTX, CC_SHA224_Init, 
	CC_SHA224_Update, CC_SHA224_Final)

CC_DIGEST_ONE_SHOT(CC_SHA256, CC_SHA256_CTX, CC_SHA256_Init, 
	CC_SHA256_Update, CC_SHA256_Final)

CC_DIGEST_ONE_SHOT(CC_SHA384, CC_SHA512_CTX, CC_SHA384_Init, 
	CC_SHA384_Update, CC_SHA384_Final)

CC_DIGEST_ONE_SHOT(CC_SHA512, CC_SHA512_CTX, CC_SHA512_Init, 
	CC_SHA512_Update, CC_SHA512_Final)
