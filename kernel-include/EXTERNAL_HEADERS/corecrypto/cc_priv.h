/*
 *  cc_priv.h
 *  corecrypto
 *
 *  Created on 12/01/2010
 *
 *  Copyright (c) 2010,2011,2012,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CC_PRIV_H_
#define _CORECRYPTO_CC_PRIV_H_

#include <corecrypto/cc.h>
#include <stdint.h>

// Fork handlers for the stateful components of corecrypto.
void cc_atfork_prepare(void);
void cc_atfork_parent(void);
void cc_atfork_child(void);

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifndef __DECONST
#define __DECONST(type, var) ((type)(uintptr_t)(const void *)(var))
#endif

/* defines the following macros :

 CC_ARRAY_LEN: returns the number of elements in an array

 CC_STORE32_BE : store 32 bit value in big endian in unaligned buffer.
 CC_STORE32_LE : store 32 bit value in little endian in unaligned buffer.
 CC_STORE64_BE : store 64 bit value in big endian in unaligned buffer.
 CC_STORE64_LE : store 64 bit value in little endian in unaligned buffer.

 CC_LOAD32_BE : load 32 bit value in big endian from unaligned buffer.
 CC_LOAD32_LE : load 32 bit value in little endian from unaligned buffer.
 CC_LOAD64_BE : load 64 bit value in big endian from unaligned buffer.
 CC_LOAD64_LE : load 64 bit value in little endian from unaligned buffer.

 CC_ROR  : Rotate Right 32 bits. Rotate count can be a variable.
 CC_ROL  : Rotate Left 32 bits. Rotate count can be a variable.
 CC_RORc : Rotate Right 32 bits. Rotate count must be a constant.
 CC_ROLc : Rotate Left 32 bits. Rotate count must be a constant.

 CC_ROR64  : Rotate Right 64 bits. Rotate count can be a variable.
 CC_ROL64  : Rotate Left 64 bits. Rotate count can be a variable.
 CC_ROR64c : Rotate Right 64 bits. Rotate count must be a constant.
 CC_ROL64c : Rotate Left 64 bits. Rotate count must be a constant.

 CC_BSWAP  : byte swap a 32 bits variable.

 CC_H2BE32 : convert a 32 bits value between host and big endian order.
 CC_H2LE32 : convert a 32 bits value between host and little endian order.

 CC_BSWAP64  : byte swap a 64 bits variable

 CC_READ_LE32 : read a 32 bits little endian value

 CC_WRITE_LE32 : write a 32 bits little endian value
 CC_WRITE_LE64 : write a 64 bits little endian value

 CC_H2BE64 : convert a 64 bits value between host and big endian order
 CC_H2LE64 : convert a 64 bits value between host and little endian order

*/

// <rdar://problem/40683103> RTKitOSPlatform should replace CC_MEMCPY with memcpy
#define CC_MEMCPY(D,S,L) cc_memcpy((D),(S),(L))
#define CC_MEMMOVE(D,S,L) cc_memmove((D),(S),(L))
#define CC_MEMSET(D,V,L) cc_memset((D),(V),(L))

#if __has_builtin(__builtin___memcpy_chk) && !CC_RTKIT
#define cc_memcpy(dst, src, len) __builtin___memcpy_chk((dst), (src), (len), __builtin_object_size((dst), 1))
#define cc_memcpy_nochk(dst, src, len) __builtin___memcpy_chk((dst), (src), (len), __builtin_object_size((dst), 0))
#else
#define cc_memcpy(dst, src, len) memcpy((dst), (src), (len))
#define cc_memcpy_nochk(dst, src, len) memcpy((dst), (src), (len))
#endif

#if __has_builtin(__builtin___memmove_chk) && !CC_RTKIT
#define cc_memmove(dst, src, len) __builtin___memmove_chk((dst), (src), (len), __builtin_object_size((dst), 1))
#else
#define cc_memmove(dst, src, len) memmove((dst), (src), (len))
#endif

#if __has_builtin(__builtin___memset_chk) && !CC_RTKIT
#define cc_memset(dst, val, len) __builtin___memset_chk((dst), (val), (len), __builtin_object_size((dst), 1))
#else
#define cc_memset(dst, val, len) memset((dst), (val), (len))
#endif

#define CC_ARRAY_LEN(x) (sizeof((x))/sizeof((x)[0]))

// MARK: - Loads and Store

// MARK: -- 32 bits - little endian

// MARK: --- Default version

#define	CC_STORE32_LE(x, y) do {                                    \
    ((unsigned char *)(y))[3] = (unsigned char)(((x)>>24)&255);		\
    ((unsigned char *)(y))[2] = (unsigned char)(((x)>>16)&255);		\
    ((unsigned char *)(y))[1] = (unsigned char)(((x)>>8)&255);		\
    ((unsigned char *)(y))[0] = (unsigned char)((x)&255);			\
} while(0)

#define	CC_LOAD32_LE(x, y) do {                                     \
x = ((uint32_t)(((const unsigned char *)(y))[3] & 255)<<24) |			    \
    ((uint32_t)(((const unsigned char *)(y))[2] & 255)<<16) |			    \
    ((uint32_t)(((const unsigned char *)(y))[1] & 255)<<8)  |			    \
    ((uint32_t)(((const unsigned char *)(y))[0] & 255));				    \
} while(0)

// MARK: -- 64 bits - little endian

#define	CC_STORE64_LE(x, y) do {                                    \
    ((unsigned char *)(y))[7] = (unsigned char)(((x)>>56)&255);     \
    ((unsigned char *)(y))[6] = (unsigned char)(((x)>>48)&255);		\
    ((unsigned char *)(y))[5] = (unsigned char)(((x)>>40)&255);		\
    ((unsigned char *)(y))[4] = (unsigned char)(((x)>>32)&255);		\
    ((unsigned char *)(y))[3] = (unsigned char)(((x)>>24)&255);		\
    ((unsigned char *)(y))[2] = (unsigned char)(((x)>>16)&255);		\
    ((unsigned char *)(y))[1] = (unsigned char)(((x)>>8)&255);		\
    ((unsigned char *)(y))[0] = (unsigned char)((x)&255);			\
} while(0)

#define	CC_LOAD64_LE(x, y) do {                                     \
x = (((uint64_t)(((const unsigned char *)(y))[7] & 255))<<56) |           \
    (((uint64_t)(((const unsigned char *)(y))[6] & 255))<<48) |           \
    (((uint64_t)(((const unsigned char *)(y))[5] & 255))<<40) |           \
    (((uint64_t)(((const unsigned char *)(y))[4] & 255))<<32) |           \
    (((uint64_t)(((const unsigned char *)(y))[3] & 255))<<24) |           \
    (((uint64_t)(((const unsigned char *)(y))[2] & 255))<<16) |           \
    (((uint64_t)(((const unsigned char *)(y))[1] & 255))<<8)  |           \
    (((uint64_t)(((const unsigned char *)(y))[0] & 255)));                \
} while(0)

// MARK: -- 32 bits - big endian
// MARK: --- intel version

#if (defined(__i386__) || defined(__x86_64__)) && !defined(_MSC_VER)

#define CC_STORE32_BE(x, y)     \
    __asm__ __volatile__ (      \
    "bswapl %0     \n\t"        \
    "movl   %0,(%1)\n\t"        \
    "bswapl %0     \n\t"        \
    ::"r"(x), "r"(y))

#define CC_LOAD32_BE(x, y)      \
    __asm__ __volatile__ (      \
    "movl (%1),%0\n\t"          \
    "bswapl %0\n\t"             \
    :"=r"(x): "r"(y))

#else
// MARK: --- default version
#define	CC_STORE32_BE(x, y) do {                                \
    ((unsigned char *)(y))[0] = (unsigned char)(((x)>>24)&255);	\
    ((unsigned char *)(y))[1] = (unsigned char)(((x)>>16)&255);	\
    ((unsigned char *)(y))[2] = (unsigned char)(((x)>>8)&255);	\
    ((unsigned char *)(y))[3] = (unsigned char)((x)&255);       \
} while(0)

#define	CC_LOAD32_BE(x, y) do {                             \
x = ((uint32_t)(((const unsigned char *)(y))[0] & 255)<<24) |	    \
    ((uint32_t)(((const unsigned char *)(y))[1] & 255)<<16) |		\
    ((uint32_t)(((const unsigned char *)(y))[2] & 255)<<8)  |		\
    ((uint32_t)(((const unsigned char *)(y))[3] & 255));          \
} while(0)

#endif

// MARK: -- 64 bits - big endian

// MARK: --- intel 64 bits version

#if defined(__x86_64__) && !defined (_MSC_VER)

#define	CC_STORE64_BE(x, y)   \
__asm__ __volatile__ (        \
"bswapq %0     \n\t"          \
"movq   %0,(%1)\n\t"          \
"bswapq %0     \n\t"          \
::"r"(x), "r"(y))

#define	CC_LOAD64_BE(x, y)    \
__asm__ __volatile__ (        \
"movq (%1),%0\n\t"            \
"bswapq %0\n\t"               \
:"=r"(x): "r"(y))

#else

// MARK: --- default version

#define CC_STORE64_BE(x, y) do {                                    \
    ((unsigned char *)(y))[0] = (unsigned char)(((x)>>56)&255);		\
    ((unsigned char *)(y))[1] = (unsigned char)(((x)>>48)&255);		\
    ((unsigned char *)(y))[2] = (unsigned char)(((x)>>40)&255);		\
    ((unsigned char *)(y))[3] = (unsigned char)(((x)>>32)&255);		\
    ((unsigned char *)(y))[4] = (unsigned char)(((x)>>24)&255);		\
    ((unsigned char *)(y))[5] = (unsigned char)(((x)>>16)&255);		\
    ((unsigned char *)(y))[6] = (unsigned char)(((x)>>8)&255);		\
    ((unsigned char *)(y))[7] = (unsigned char)((x)&255);			\
} while(0)

#define	CC_LOAD64_BE(x, y) do {                                     \
x = (((uint64_t)(((const unsigned char *)(y))[0] & 255))<<56) |           \
    (((uint64_t)(((const unsigned char *)(y))[1] & 255))<<48) |           \
    (((uint64_t)(((const unsigned char *)(y))[2] & 255))<<40) |           \
    (((uint64_t)(((const unsigned char *)(y))[3] & 255))<<32) |           \
    (((uint64_t)(((const unsigned char *)(y))[4] & 255))<<24) |           \
    (((uint64_t)(((const unsigned char *)(y))[5] & 255))<<16) |           \
    (((uint64_t)(((const unsigned char *)(y))[6] & 255))<<8)  |          	\
    (((uint64_t)(((const unsigned char *)(y))[7] & 255)));	            \
} while(0)

#endif

// MARK: - 32-bit Rotates

#if defined(_MSC_VER)
// MARK: -- MSVC version

#include <stdlib.h>
#if !defined(__clang__)
 #pragma intrinsic(_lrotr,_lrotl)
#endif
#define	CC_ROR(x,n) _lrotr(x,n)
#define	CC_ROL(x,n) _lrotl(x,n)
#define	CC_RORc(x,n) _lrotr(x,n)
#define	CC_ROLc(x,n) _lrotl(x,n)

#elif (defined(__i386__) || defined(__x86_64__))
// MARK: -- intel asm version

CC_INLINE uint32_t CC_ROL(uint32_t word, int i)
{
    __asm__ ("roll %%cl,%0"
         :"=r" (word)
         :"0" (word),"c" (i));
    return word;
}

CC_INLINE uint32_t CC_ROR(uint32_t word, int i)
{
    __asm__ ("rorl %%cl,%0"
         :"=r" (word)
         :"0" (word),"c" (i));
    return word;
}

/* Need to be a macro here, because 'i' is an immediate (constant) */
#define CC_ROLc(word, i)                \
({  uint32_t _word=(word);              \
    __asm__ __volatile__ ("roll %2,%0"  \
        :"=r" (_word)                   \
        :"0" (_word),"I" (i));          \
    _word;                              \
})


#define CC_RORc(word, i)                \
({  uint32_t _word=(word);              \
    __asm__ __volatile__ ("rorl %2,%0"  \
        :"=r" (_word)                   \
        :"0" (_word),"I" (i));          \
    _word;                              \
})

#else

// MARK: -- default version

CC_INLINE uint32_t CC_ROL(uint32_t word, int i)
{
    return ( (word<<(i&31)) | (word>>(32-(i&31))) );
}

CC_INLINE uint32_t CC_ROR(uint32_t word, int i)
{
    return ( (word>>(i&31)) | (word<<(32-(i&31))) );
}

#define	CC_ROLc(x, y) CC_ROL(x, y)
#define	CC_RORc(x, y) CC_ROR(x, y)

#endif

// MARK: - 64 bits rotates

#if defined(__x86_64__) && !defined(_MSC_VER) //clang _MSVC doesn't support GNU-style inline assembly
// MARK: -- intel 64 asm version

CC_INLINE uint64_t CC_ROL64(uint64_t word, int i)
{
    __asm__("rolq %%cl,%0"
        :"=r" (word)
        :"0" (word),"c" (i));
    return word;
}

CC_INLINE uint64_t CC_ROR64(uint64_t word, int i)
{
    __asm__("rorq %%cl,%0"
        :"=r" (word)
        :"0" (word),"c" (i));
    return word;
}

/* Need to be a macro here, because 'i' is an immediate (constant) */
#define CC_ROL64c(word, i)      \
({                              \
    uint64_t _word=(word);      \
    __asm__("rolq %2,%0"        \
        :"=r" (_word)           \
        :"0" (_word),"J" (i));  \
    _word;                      \
})

#define CC_ROR64c(word, i)      \
({                              \
    uint64_t _word=(word);      \
    __asm__("rorq %2,%0"        \
        :"=r" (_word)           \
        :"0" (_word),"J" (i));  \
    _word;                      \
})


#else /* Not x86_64  */

// MARK: -- default C version

CC_INLINE uint64_t CC_ROL64(uint64_t word, int i)
{
    return ( (word<<(i&63)) | (word>>(64-(i&63))) );
}

CC_INLINE uint64_t CC_ROR64(uint64_t word, int i)
{
    return ( (word>>(i&63)) | (word<<(64-(i&63))) );
}

#define	CC_ROL64c(x, y) CC_ROL64(x, y)
#define	CC_ROR64c(x, y) CC_ROR64(x, y)

#endif


// MARK: - Byte Swaps

#if __has_builtin(__builtin_bswap32)
#define CC_BSWAP32(x) __builtin_bswap32(x)
#else
CC_INLINE uint32_t CC_BSWAP32(uint32_t x)
{
    return
        ((x & 0xff000000) >> 24) |
        ((x & 0x00ff0000) >>  8) |
        ((x & 0x0000ff00) <<  8) |
        ((x & 0x000000ff) << 24);
}
#endif

#if __has_builtin(__builtin_bswap64)
#define CC_BSWAP64(x) __builtin_bswap64(x)
#else
CC_INLINE uint64_t CC_BSWAP64(uint64_t x)
{
    return
        ((x & 0xff00000000000000ULL) >> 56) |
        ((x & 0x00ff000000000000ULL) >> 40) |
        ((x & 0x0000ff0000000000ULL) >> 24) |
        ((x & 0x000000ff00000000ULL) >>  8) |
        ((x & 0x00000000ff000000ULL) <<  8) |
        ((x & 0x0000000000ff0000ULL) << 24) |
        ((x & 0x000000000000ff00ULL) << 40) |
        ((x & 0x00000000000000ffULL) << 56);
}
#endif

#ifdef __LITTLE_ENDIAN__
#define CC_H2BE32(x) CC_BSWAP32(x)
#define CC_H2LE32(x) (x)
#define CC_H2BE64(x) CC_BSWAP64(x)
#define CC_H2LE64(x) (x)
#else
#define CC_H2BE32(x) (x)
#define CC_H2LE32(x) CC_BSWAP32(x)
#define CC_H2BE64(x) (x)
#define CC_H2LE64(x) CC_BSWAP64(x)
#endif

#define	CC_READ_LE32(ptr) \
( (uint32_t)( \
((uint32_t)((const uint8_t *)(ptr))[0]) | \
(((uint32_t)((const uint8_t *)(ptr))[1]) <<  8) | \
(((uint32_t)((const uint8_t *)(ptr))[2]) << 16) | \
(((uint32_t)((const uint8_t *)(ptr))[3]) << 24)))

#define	CC_WRITE_LE32(ptr, x) \
do { \
((uint8_t *)(ptr))[0] = (uint8_t)( (x)        & 0xFF); \
((uint8_t *)(ptr))[1] = (uint8_t)(((x) >>  8) & 0xFF); \
((uint8_t *)(ptr))[2] = (uint8_t)(((x) >> 16) & 0xFF); \
((uint8_t *)(ptr))[3] = (uint8_t)(((x) >> 24) & 0xFF); \
} while(0)

#define	CC_WRITE_LE64(ptr, x) \
do { \
((uint8_t *)(ptr))[0] = (uint8_t)( (x)        & 0xFF); \
((uint8_t *)(ptr))[1] = (uint8_t)(((x) >>  8) & 0xFF); \
((uint8_t *)(ptr))[2] = (uint8_t)(((x) >> 16) & 0xFF); \
((uint8_t *)(ptr))[3] = (uint8_t)(((x) >> 24) & 0xFF); \
((uint8_t *)(ptr))[4] = (uint8_t)(((x) >> 32) & 0xFF); \
((uint8_t *)(ptr))[5] = (uint8_t)(((x) >> 40) & 0xFF); \
((uint8_t *)(ptr))[6] = (uint8_t)(((x) >> 48) & 0xFF); \
((uint8_t *)(ptr))[7] = (uint8_t)(((x) >> 56) & 0xFF); \
} while(0)

/* extract a byte portably */
#ifdef _MSC_VER
#define cc_byte(x, n) ((unsigned char)((x) >> (8 * (n))))
#else
#define cc_byte(x, n) (((x) >> (8 * (n))) & 255)
#endif

/* Count leading zeros (for nonzero inputs) */

/*
 *  On i386 and x86_64, we know clang and GCC will generate BSR for
 *  __builtin_clzl.  This instruction IS NOT constant time on all micro-
 *  architectures, but it *is* constant time on all micro-architectures that
 *  have been used by Apple, and we expect that to continue to be the case.
 *
 *  When building for x86_64h with clang, this produces LZCNT, which is exactly
 *  what we want.
 *
 *  On arm and arm64, we know that clang and GCC generate the constant-time CLZ
 *  instruction from __builtin_clzl( ).
 */

#if defined(_WIN32)
/* We use the Windows implementations below. */
#elif defined(__x86_64__) || defined(__i386__) || defined(__arm64__) || defined(__arm__)
/* We use a thought-to-be-good version of __builtin_clz. */
#elif defined __GNUC__
#warning Using __builtin_clz() on an unknown architecture; it may not be constant-time.
/* If you find yourself seeing this warning, file a radar for someone to
 * check whether or not __builtin_clz() generates a constant-time
 * implementation on the architecture you are targeting.  If it does, append
 * the name of that architecture to the list of "safe" architectures above.  */                                          */
#endif


#if defined(_WIN32)

#include <windows.h>
#include <intrin.h>

CC_INLINE CC_CONST unsigned clz64_win(uint64_t value)
{
    DWORD leading_zero;
    _BitScanReverse64(&leading_zero, value);
    return 63 - leading_zero;
}


CC_INLINE CC_CONST unsigned clz32_win(uint32_t value)
{
    DWORD leading_zero;
    _BitScanReverse(&leading_zero, value);
    return 31 - leading_zero;
}

#endif

CC_INLINE CC_CONST unsigned cc_clz32_fallback(uint32_t data)
{
    unsigned int b = 0;
    unsigned int bit = 0;
    // Work from LSB to MSB
    for (int i = 0; i < 32; i++) {
        bit = (data >> i) & 1;
        // If the bit is 0, update the "leading bits are zero" counter "b".
        b += (1 - bit);
        /* If the bit is 0, (bit - 1) is 0xffff... therefore b is retained.
         * If the bit is 1, (bit - 1) is 0 therefore b is set to 0.
         */
        b &= (bit - 1);
    }
    return b;
}

CC_INLINE CC_CONST unsigned cc_clz64_fallback(uint64_t data)
{
    unsigned int b = 0;
    unsigned int bit = 0;
    // Work from LSB to MSB
    for (int i = 0; i < 64; i++) {
        bit = (data >> i) & 1;
        // If the bit is 0, update the "leading bits are zero" counter.
        b += (1 - bit);
        /* If the bit is 0, (bit - 1) is 0xffff... therefore b is retained.
         * If the bit is 1, (bit - 1) is 0 therefore b is set to 0.
         */
        b &= (bit - 1);
    }
    return b;
}

/*!
  @function cc_clz32
  @abstract Count leading zeros of a nonzero 32-bit value

  @param data A nonzero 32-bit value

  @result Count of leading zeros of @p data

  @discussion @p data is assumed to be nonzero.
*/
CC_INLINE CC_CONST unsigned cc_clz32(uint32_t data) {
#if defined(_WIN32)
    return clz32_win(data);
#elif defined(__x86_64__) || defined(__i386__) || defined(__arm64__) || defined(__arm__) || defined(__GNUC__)
    cc_static_assert(sizeof(unsigned) == 4, "clz relies on an unsigned int being 4 bytes");
    return (unsigned)__builtin_clz(data);
#else
    return cc_clz32_fallback(data);
#endif
}

/*!
  @function cc_clz64
  @abstract Count leading zeros of a nonzero 64-bit value

  @param data A nonzero 64-bit value

  @result Count of leading zeros of @p data

  @discussion @p data is assumed to be nonzero.
*/
CC_INLINE CC_CONST unsigned cc_clz64(uint64_t data) {
#if defined(_WIN32)
    return clz64_win(data);
#elif defined(__x86_64__) || defined(__i386__) || defined(__arm64__) || defined(__arm__) || defined(__GNUC__)
    return (unsigned)__builtin_clzll(data);
#else
    return cc_clz64_fallback(data);
#endif
}

/* HEAVISIDE_STEP (shifted by one)
   function f(x): x->0, when x=0
                  x->1, when x>0
   Can also be seen as a bitwise operation:
      f(x): x -> y
        y[0]=(OR x[i]) for all i (all bits)
        y[i]=0 for all i>0
   Run in constant time (log2(<bitsize of x>))
   Useful to run constant time checks
*/
#define CC_HEAVISIDE_STEP(r, s) {                       \
    const uint64_t _s = (uint64_t)s;                    \
    const uint64_t _t = (_s & 0xffffffff) | (_s >> 32); \
    r = (__typeof__(r))((0xffffffff + _t) >> 32);       \
}

/* Return 1 if x mod 4 =1,2,3, 0 otherwise */
#define CC_CARRY_2BITS(x) (((x>>1) | x) & 0x1)
#define CC_CARRY_3BITS(x) (((x>>2) | (x>>1) | x) & 0x1)

/* Set a variable to the biggest power of 2 which can be represented */
#define MAX_POWER_OF_2(x)   ((__typeof__(x))1<<(8*sizeof(x)-1))
#define cc_ceiling(a,b)  (((a)+((b)-1))/(b))
#define CC_BITLEN_TO_BYTELEN(x) cc_ceiling((x), 8)

/*!
 @brief     cc_muxp(s, a, b) is equivalent to z = s ? a : b, but it executes in constant time
 @param a	input pointer
 @param b	input pointer
 @param s	The selection parameter s must be 0 or 1. if s is integer 1 a is returned. If s is integer 0, b is returned. Otherwise, the output is undefined.
 @return    Returns a, if s is 1 and b if s is 0
 */
void *cc_muxp(int s, const void *a, const void *b);

/*!
 @brief     CC_MUXU(r, s, a, b) is equivalent to r = s ? a : b, but executes in constant time
 @param a   Input a
 @param b   Input b
 @param s   Selection parameter s. Must be 0 or 1.
 @param r   Output, set to a if s=1, or b if s=0.
 */
#define CC_MUXU(r, s, a, b)                                      \
{                                                                \
    __typeof__(r) _cond = ((__typeof__(r))(s)-(__typeof__(r))1); \
    r = (~_cond&(a))|(_cond&(b));                                \
}

#define CC_PROVIDES_ABORT (!(CC_USE_SEPROM || CC_USE_S3 || CC_BASEBAND || CC_EFI || CC_IBOOT || CC_RTKITROM))

/*!
 @function cc_abort
 @abstract Abort execution unconditionally
 */
CC_NORETURN
void cc_abort(const char *msg);

/*!
  @function cc_try_abort
  @abstract Abort execution iff the platform provides a function like @p abort() or @p panic()

  @discussion If the platform does not provide a means to abort execution, this function does nothing; therefore, callers should return an error code after calling this function.
*/
#if CC_PROVIDES_ABORT

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

CC_INLINE
void cc_try_abort(const char *msg)
{
    cc_abort(msg);
}

#pragma clang diagnostic pop

#else

CC_INLINE
void cc_try_abort(CC_UNUSED const char *msg)
{

}

#endif

/*
  Unfortunately, since we export this symbol, this declaration needs
  to be in a public header to satisfy TAPI.

  See fipspost_trace_priv.h for more details.
*/
extern const void *fipspost_trace_vtable;

#endif /* _CORECRYPTO_CC_PRIV_H_ */
