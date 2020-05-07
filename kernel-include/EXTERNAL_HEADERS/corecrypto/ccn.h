/*
 *  ccn.h
 *  corecrypto
 *
 *  Created on 11/16/2010
 *
 *  Copyright (c) 2010,2011,2012,2013,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCN_H_
#define _CORECRYPTO_CCN_H_

#include <corecrypto/cc.h>
#include <stdint.h>
#include <stdarg.h>

typedef uint8_t cc_byte;
typedef size_t  cc_size;

#if  CCN_UNIT_SIZE == 8
typedef uint64_t cc_unit;          // 64 bit unit
typedef int64_t  cc_int;
#define CCN_LOG2_BITS_PER_UNIT  6  // 2^6 = 64 bits
#define CC_UNIT_C(x) UINT64_C(x)
 #if  CCN_UINT128_SUPPORT_FOR_64BIT_ARCH
   typedef unsigned cc_dunit __attribute__((mode(TI)));         // 128 bit double width unit
   typedef   signed cc_dint  __attribute__((mode(TI)));
 #else
   typedef struct cc_dunit {
    uint64_t l; //do not change the order of the variables. cc_dunit must be little endian
    uint64_t h;
   } cc_dunit;

   typedef struct cc_dint {
    uint64_t l;
    uint64_t h;
   } cc_dint;
 #endif

#elif  CCN_UNIT_SIZE == 4
typedef uint32_t cc_unit;          // 32 bit unit
typedef uint64_t cc_dunit;         // 64 bit double width unit
typedef int64_t cc_dint;
typedef int32_t cc_int;
#define CCN_LOG2_BITS_PER_UNIT  5  // 2^5 = 32 bits
#define CC_UNIT_C(x) UINT32_C(x)

#elif CCN_UNIT_SIZE == 2
typedef uint16_t cc_unit;          // 16 bit unit
typedef uint32_t cc_dunit;         // 32 bit double width unit
#define CCN_LOG2_BITS_PER_UNIT  4  // 2^4 = 16 bits
#define CC_UNIT_C(x) UINT16_C(x)

#elif CCN_UNIT_SIZE == 1
typedef uint8_t cc_unit;           // 8 bit unit
typedef uint16_t cc_dunit;         // 16 bit double width unit
#define CCN_LOG2_BITS_PER_UNIT  3  // 2^3 = 8 bits
#define CC_UNIT_C(x) UINT8_C(x)

#else
#error invalid CCN_UNIT_SIZE
#endif

#define CCN_UNIT_BITS  (sizeof(cc_unit) * 8)
#define CCN_UNIT_MASK  ((cc_unit)~0)
#define CCN_UNIT_LOWER_HALF_MASK  ((CCN_UNIT_MASK) >> (CCN_UNIT_BITS/2))
#define CCN_UNIT_UPPER_HALF_MASK  (~CCN_UNIT_LOWER_HALF_MASK)

typedef struct {
    cc_unit *start;      // First cc_unit of the workspace
    cc_unit *end;        // address and beyond NOT TO BE TOUCHED
} cc_ws,*cc_ws_t;

/* Conversions between n sizeof and bits */

/* Returns the sizeof a ccn vector of length _n_ units. */
#define ccn_sizeof_n(_n_)  (sizeof(cc_unit) * (_n_))

/* Returns the count (n) of a ccn vector that can represent _bits_. */
#define ccn_nof(_bits_)  (((_bits_) + CCN_UNIT_BITS - 1) >> CCN_LOG2_BITS_PER_UNIT)

/* Returns the sizeof a ccn vector that can represent _bits_. */
#define ccn_sizeof(_bits_)  (ccn_sizeof_n(ccn_nof(_bits_)))

/* Returns the count (n) of a ccn vector that can represent _size_ bytes. */
#define ccn_nof_size(_size_)  (((_size_) + CCN_UNIT_SIZE - 1) / CCN_UNIT_SIZE)

#define ccn_nof_sizeof(_expr_) ccn_nof_size(sizeof (_expr_))

/* Return the max number of bits a ccn vector of _n_ units can hold. */
#define ccn_bitsof_n(_n_)  ((_n_) * CCN_UNIT_BITS)

/* Return the max number of bits a ccn vector of _size_ bytes can hold. */
#define ccn_bitsof_size(_size_)  ((_size_) * 8)

/* Return the size of a ccn of size bytes in bytes. */
#define ccn_sizeof_size(_size_)  ccn_sizeof_n(ccn_nof_size(_size_))

/* Returns the value of bit _k_ of _ccn_, both are only evaluated once.  */
#define ccn_bit(_ccn_, _k_) ({__typeof__ (_k_) __k = (_k_); \
    1 & ((_ccn_)[ __k >> CCN_LOG2_BITS_PER_UNIT] >> (__k & (CCN_UNIT_BITS - 1)));})

/* Set the value of bit _k_ of _ccn_ to the value _v_  */
#define ccn_set_bit(_ccn_, _k_, _v_) ({__typeof__ (_k_) __k = (_k_);        \
    if (_v_)                                                                \
        (_ccn_)[ __k >> CCN_LOG2_BITS_PER_UNIT] |= CC_UNIT_C(1) << (__k & (CCN_UNIT_BITS - 1));     \
    else                                                                    \
        (_ccn_)[ __k >> CCN_LOG2_BITS_PER_UNIT] &= ~(CC_UNIT_C(1) << (__k & (CCN_UNIT_BITS - 1)));  \
    })

/* Macros for making ccn constants.  You must use list of CCN64_C() instances
 separated by commas, with an optional smaller sized CCN32_C, CCN16_C, or
 CCN8_C() instance at the end of the list, when making macros to declare
 larger sized constants. */
#define CCN8_C(a0) CC_UNIT_C(0x##a0)

#if CCN_UNIT_SIZE >= 2
#define CCN16_C(a1,a0) CC_UNIT_C(0x##a1##a0)
#define ccn16_v(a0)  (a0)
#elif CCN_UNIT_SIZE == 1
#define CCN16_C(a1,a0) CCN8_C(a0),CCN8_C(a1)
#define ccn16_v(a0)  (a0 & UINT8_C(0xff)),(a0 >> 8)
#endif

#if CCN_UNIT_SIZE >= 4
#define CCN32_C(a3,a2,a1,a0) CC_UNIT_C(0x##a3##a2##a1##a0)
#define ccn32_v(a0)  (a0)
#else
#define CCN32_C(a3,a2,a1,a0) CCN16_C(a1,a0),CCN16_C(a3,a2)
#define ccn32_v(a0)  ccn16_v(a0 & UINT16_C(0xffff)),ccn16_v(a0 >> 16)
#endif

#if CCN_UNIT_SIZE == 8
#define CCN64_C(a7,a6,a5,a4,a3,a2,a1,a0) CC_UNIT_C(0x##a7##a6##a5##a4##a3##a2##a1##a0)
#define CCN40_C(a4,a3,a2,a1,a0) CC_UNIT_C(0x##a4##a3##a2##a1##a0)
#define ccn64_v(a0)  (a0)
//#define ccn64_32(a1,a0)  ((a1 << 32) | a0)
//#define ccn_uint64(a,i) (a[i])
#else
#define CCN64_C(a7,a6,a5,a4,a3,a2,a1,a0) CCN32_C(a3,a2,a1,a0),CCN32_C(a7,a6,a5,a4)
#define CCN40_C(a4,a3,a2,a1,a0) CCN32_C(a3,a2,a1,a0),CCN8_C(a4)
#define ccn64_v(a0)  ccn32_v((uint64_t)a0 & UINT32_C(0xffffffff)),ccn32_v((uint64_t)a0 >> 32)
//#define ccn64_32(a1,a0)  ccn32_v(a0),ccn32_v(a1)
//#define ccn_uint64(a,i) ((uint64_t)ccn_uint32(a, i << 1 + 1) << 32 | (uint64_t)ccn_uint32(a, i << 1))
#endif

/* Macro's for reading uint32_t and uint64_t from ccns, the index is in 32 or
   64 bit units respectively. */
#if CCN_UNIT_SIZE == 8
/* #define ccn_uint16(a,i) ((i & 3) == 3 ? ((uint16_t)(a[i >> 2] >> 48)) : \
     (i & 3) == 2 ? ((uint16_t)(a[i >> 2] >> 32) & UINT16_C(0xffff)) : \
     (i & 3) == 1 ? ((uint16_t)(a[i >> 2] >> 16) & UINT16_C(0xffff)) : \
     ((uint16_t)(a[i >> 1] & UINT16_C(0xffff))))
*/
//#define ccn_uint32(a,i) (i & 1 ? ((uint32_t)(a[i >> 1] >> 32)) : ((uint32_t)(a[i >> 1] & UINT32_C(0xffffffff))))
#elif CCN_UNIT_SIZE == 4
//#define ccn16_v(a0)  (a0)
//#define ccn32_v(a0)  (a0)
//#define ccn_uint16(a,i) (i & 1 ? ((uint16_t)(a[i >> 1] >> 16)) : ((uint16_t)(a[i >> 1] & UINT16_C(0xffff))))
//#define ccn_uint32(a,i) (a[i])
#elif CCN_UNIT_SIZE == 2
//#define ccn16_v(a0)  (a0)
//#define ccn32_v(a0,a1)  (a1,a0)
//#define ccn_uint16(a,i) (a[i])
//#define ccn_uint32(a,i) (((uint32_t)a[i << 1 + 1]) << 16 | (uint32_t)a[i << 1]))
#elif CCN_UNIT_SIZE == 1
//#define ccn16_v(a0)  (a0 & UINT8_C(0xff)),(a0 >> 8)
//#define ccn_uint16(a,i) ((uint16_t)((a[i << 1 + 1] << 8) | a[i << 1]))
//#define ccn_uint32(a,i) ((uint32_t)ccn_uint16(a, i << 1 + 1) << 16 | (uint32_t)ccn_uint16(a, i << 1))
#endif

/* Macro's for reading uint32_t and uint64_t from ccns, the index is in 32 or
 64 bit units respectively. */
#if CCN_UNIT_SIZE == 8

#define ccn64_32(a1,a0) (((const cc_unit)a1) << 32 | ((const cc_unit)a0))
#define ccn32_32(a0) a0
#if __LITTLE_ENDIAN__
#define ccn32_32_parse(p,i) (((const uint32_t *)p)[i])
#else
#define ccn32_32_parse(p,i) (((const uint32_t *)p)[i^1])
#endif
#define ccn32_32_null 0

#define ccn64_64(a0) a0
#define ccn64_64_parse(p,i) p[i]
#define ccn64_64_null 0

#elif CCN_UNIT_SIZE == 4

#define ccn32_32(a0) a0
#define ccn32_32_parse(p,i) p[i]
#define ccn32_32_null 0
#define ccn64_32(a1,a0) ccn32_32(a0),ccn32_32(a1)

#define ccn64_64(a1,a0) a0,a1
#define ccn64_64_parse(p,i) p[1+(i<<1)],p[i<<1]
#define ccn64_64_null 0,0

#elif CCN_UNIT_SIZE == 2

#define ccn32_32(a1,a0) a0,a1
#define ccn32_32_parse(p,i) p[1+(i<<1)],p[i<<1]
#define ccn32_32_null 0,0
#define ccn64_32(a3,a2,a1,a0) ccn32_32(a1,a0),ccn32_32(a3,a2)

#define ccn64_64(a3,a2,a1,a0) a0,a1,a2,a3
#define ccn64_64_parse(p,i) p[3+(i<<2)],p[2+(i<<2)],p[1+(i<<2)],p[i<<2]
#define ccn64_64_null 0,0,0,0

#elif CCN_UNIT_SIZE == 1

#define ccn32_32(a3,a2,a1,a0) a0,a1,a2,a3
#define ccn32_32_parse(p,i) p[3+(i<<2)],p[2+(i<<2)],p[1+(i<<2)],p[i<<2]
#define ccn32_32_null 0,0,0,0
#define ccn64_32(a7,a6,a5,a4,a3,a2,a1,a0) ccn32_32(a3,a2,a1,a0),ccn32_32(a7,a6,a5,a4)

#define ccn64_64(a7,a6,a5,a4,a3,a2,a1,a0) a0,a1,a2,a3,a4,a5,a6,a7
#define ccn64_64_parse(p,i)  p[7+(i<<3)],p[6+(i<<3)],p[5+(i<<3)],p[4+(i<<3)],p[3+(i<<3)],p[2+(i<<3)],p[1+(i<<3)],p[i<<3]
#define ccn64_64_null  0,0,0,0,0,0,0,0

#endif


/* Macros to construct fixed size ccn arrays from 64 or 32 bit quantities. */
#define ccn192_64(a2,a1,a0) ccn64_64(a0),ccn64_64(a1),ccn64_64(a2)
#define ccn192_32(a5,a4,a3,a2,a1,a0) ccn64_32(a1,a0),ccn64_32(a3,a2),ccn64_32(a5,a4)
#define ccn224_32(a6,a5,a4,a3,a2,a1,a0) ccn64_32(a1,a0),ccn64_32(a3,a2),ccn64_32(a5,a4),ccn32_32(a6)
#define ccn256_32(a7,a6,a5,a4,a3,a2,a1,a0) ccn64_32(a1,a0),ccn64_32(a3,a2),ccn64_32(a5,a4),ccn64_32(a7,a6)
#define ccn384_32(a11,a10,a9,a8,a7,a6,a5,a4,a3,a2,a1,a0) ccn64_32(a1,a0),ccn64_32(a3,a2),ccn64_32(a5,a4),ccn64_32(a7,a6),ccn64_32(a9,a8),ccn64_32(a11,a10)


#define CCN192_C(c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0) \
    CCN64_C(a7,a6,a5,a4,a3,a2,a1,a0),\
    CCN64_C(b7,b6,b5,b4,b3,b2,b1,b0),\
    CCN64_C(c7,c6,c5,c4,c3,c2,c1,c0)

#define CCN200_C(d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0) \
    CCN192_C(c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0),\
    CCN8_C(d0)

#define CCN224_C(d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0) \
    CCN192_C(c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0),\
    CCN32_C(d3,d2,d1,d0)

#define CCN232_C(d4,d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0) \
    CCN192_C(c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0),\
    CCN40_C(d4,d3,d2,d1,d0)

#define CCN256_C(d7,d6,d5,d4,d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0) \
    CCN192_C(c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0),\
    CCN64_C(d7,d6,d5,d4,d3,d2,d1,d0)

#define CCN264_C(e0,d7,d6,d5,d4,d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0) \
    CCN256_C(d7,d6,d5,d4,d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0),\
    CCN8_C(e0)

#define CCN384_C(f7,f6,f5,f4,f3,f2,f1,f0,e7,e6,e5,e4,e3,e2,e1,e0,d7,d6,d5,d4,d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0) \
    CCN256_C(d7,d6,d5,d4,d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0),\
    CCN64_C(e7,e6,e5,e4,e3,e2,e1,e0),\
    CCN64_C(f7,f6,f5,f4,f3,f2,f1,f0)

#define CCN392_C(g0,f7,f6,f5,f4,f3,f2,f1,f0,e7,e6,e5,e4,e3,e2,e1,e0,d7,d6,d5,d4,d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0) \
    CCN384_C(f7,f6,f5,f4,f3,f2,f1,f0,e7,e6,e5,e4,e3,e2,e1,e0,d7,d6,d5,d4,d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0),\
    CCN8_C(g0)

#define CCN528_C(i1,i0,h7,h6,h5,h4,h3,h2,h1,h0,g7,g6,g5,g4,g3,g2,g1,g0,f7,f6,f5,f4,f3,f2,f1,f0,e7,e6,e5,e4,e3,e2,e1,e0,d7,d6,d5,d4,d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0) \
    CCN256_C(d7,d6,d5,d4,d3,d2,d1,d0,c7,c6,c5,c4,c3,c2,c1,c0,b7,b6,b5,b4,b3,b2,b1,b0,a7,a6,a5,a4,a3,a2,a1,a0),\
    CCN256_C(h7,h6,h5,h4,h3,h2,h1,h0,g7,g6,g5,g4,g3,g2,g1,g0,f7,f6,f5,f4,f3,f2,f1,f0,e7,e6,e5,e4,e3,e2,e1,e0),\
    CCN16_C(i1,i0)

#define CCN192_N  ccn_nof(192)
#define CCN224_N  ccn_nof(224)
#define CCN256_N  ccn_nof(256)
#define CCN384_N  ccn_nof(384)
#define CCN512_N  ccn_nof(512)
#define CCN521_N  ccn_nof(521)

/* Return the number of used units after stripping leading 0 units.  */
CC_PURE CC_NONNULL((2))
cc_size ccn_n(cc_size n, const cc_unit *s) __asm__("_ccn_n");

/* s >> k -> r return bits shifted out of least significant word in the higest order bits of
 the retuned value. For example if CCN_UNIT_SIZE == 1, then (0b1101 1110)>>4 returns (0b1110 0000)
 and sets r==(0b0000 1101).
 { N bit, scalar -> N bit } N = n * sizeof(cc_unit) * 8
 the _multi version doesn't return the shifted bits, but does support multiple
 word shifts.  */
CC_NONNULL((2, 3))
cc_unit ccn_shift_right(cc_size n, cc_unit *r, const cc_unit *s, size_t k) __asm__("_ccn_shift_right");

/* s == 0 -> return 0 | s > 0 -> return index (starting at 1) of most
 * significant bit that is 1.
 * { N bit } N = n * sizeof(cc_unit) * 8
 *
 * Runs in constant time, independent of the value of `s`.
 */
CC_NONNULL((2))
size_t ccn_bitlen(cc_size n, const cc_unit *s);

/* s == 0 -> return true | s != 0 -> return false
 { N bit } N = n * sizeof(cc_unit) * 8 */
#define ccn_is_zero(_n_, _s_) (!ccn_n(_n_, _s_))

/* s == 1 -> return true | s != 1 -> return false
 { N bit } N = n * sizeof(cc_unit) * 8 */
#define ccn_is_one(_n_, _s_) (ccn_n(_n_, _s_) == 1 && _s_[0] == 1)

#define ccn_is_zero_or_one(_n_, _s_) (((_n_)==0) || ((ccn_n(_n_, _s_) <= 1) && (_s_[0] <= 1)))

/* s < t -> return - 1 | s == t -> return 0 | s > t -> return 1
 { N bit, N bit -> int } N = n * sizeof(cc_unit) * 8 */
CC_PURE CC_NONNULL((2, 3))
int ccn_cmp(cc_size n, const cc_unit *s, const cc_unit *t) __asm__("_ccn_cmp");

/* s < t -> return - 1 | s == t -> return 0 | s > t -> return 1
 { N bit, M bit -> int } N = ns * sizeof(cc_unit) * 8  M = nt * sizeof(cc_unit) * 8 */
CC_INLINE CC_NONNULL((2, 4))
int ccn_cmpn(cc_size ns, const cc_unit *s,
             cc_size nt, const cc_unit *t) {
    if (ns > nt) {
        return 1;
    } else if (ns < nt) {
        return -1;
    }
    return ccn_cmp(ns, s, t);
}

/* s - t -> r return 1 iff t > s
 { N bit, N bit -> N bit } N = n * sizeof(cc_unit) * 8 */
CC_NONNULL((2, 3, 4))
cc_unit ccn_sub(cc_size n, cc_unit *r, const cc_unit *s, const cc_unit *t) __asm__("_ccn_sub");

/* s - v -> r return 1 iff v > s return 0 otherwise.
 { N bit, sizeof(cc_unit) * 8 bit -> N bit } N = n * sizeof(cc_unit) * 8 */
CC_NONNULL((2, 3))
cc_unit ccn_sub1(cc_size n, cc_unit *r, const cc_unit *s, cc_unit v);

/* s - t -> r return 1 iff t > s
 { N bit, NT bit -> N bit  NT <= N} N = n * sizeof(cc_unit) * 8 */
CC_INLINE
CC_NONNULL((2, 3, 5))
cc_unit ccn_subn(cc_size n, cc_unit *r, const cc_unit *s,
             cc_size nt, const cc_unit *t) {
    assert(n >= nt);
    return ccn_sub1(n - nt, r + nt, s + nt, ccn_sub(nt, r, s, t));
}


/* s + t -> r return carry if result doesn't fit in n bits.
 { N bit, N bit -> N bit } N = n * sizeof(cc_unit) * 8 */
CC_NONNULL((2, 3, 4))
cc_unit ccn_add(cc_size n, cc_unit *r, const cc_unit *s, const cc_unit *t) __asm__("_ccn_add");

/* s + v -> r return carry if result doesn't fit in n bits.
 { N bit, sizeof(cc_unit) * 8 bit -> N bit } N = n * sizeof(cc_unit) * 8 */
CC_NONNULL((2, 3))
cc_unit ccn_add1(cc_size n, cc_unit *r, const cc_unit *s, cc_unit v);

/* s + t -> r return carry if result doesn't fit in n bits
 { N bit, NT bit -> N bit  NT <= N} N = n * sizeof(cc_unit) * 8 */
CC_INLINE
CC_NONNULL((2, 3, 5))
cc_unit ccn_addn(cc_size n, cc_unit *r, const cc_unit *s,
                 cc_size nt, const cc_unit *t) {
    assert(n >= nt);
    return ccn_add1(n - nt, r + nt, s + nt, ccn_add(nt, r, s, t));
}


/* s * t -> r_2n                   r_2n must not overlap with s nor t
 { n bit, n bit -> 2 * n bit } n = count * sizeof(cc_unit) * 8
 { N bit, N bit -> 2N bit } N = ccn_bitsof(n) */
CC_NONNULL((2, 3, 4))
void ccn_mul(cc_size n, cc_unit *r_2n, const cc_unit *s, const cc_unit *t) __asm__("_ccn_mul");

/* s[0..n) * v -> r[0..n)+return value
 { N bit, sizeof(cc_unit) * 8 bit -> N + sizeof(cc_unit) * 8 bit } N = n * sizeof(cc_unit) * 8 */
CC_NONNULL((2, 3))
cc_unit ccn_mul1(cc_size n, cc_unit *r, const cc_unit *s, const cc_unit v);

/* s[0..n) * v + r[0..n) -> r[0..n)+return value
 { N bit, sizeof(cc_unit) * 8 bit -> N + sizeof(cc_unit) * 8 bit } N = n * sizeof(cc_unit) * 8 */
CC_NONNULL((2, 3))
cc_unit ccn_addmul1(cc_size n, cc_unit *r, const cc_unit *s, const cc_unit v);


/*!
 @function   ccn_read_uint
 @abstract   Copy big endian integer and represent it in cc_units

 @param n           Input allocated size of the cc_unit output array r
 @param r           Ouput cc_unit array for unsigned integer
 @param data_nbytes Input byte size of data
 @param data        Input unsigned integer represented in big endian

 @result r is initialized with the big unsigned number

 @return 0 if no error, !=0 if the big number cannot be represented in the allocated cc_unit array.

 @discussion The execution pattern of this function depends on both n and data_nbytes but not on data values except the handling
 of the error case.
 */

CC_NONNULL((2, 4))
int ccn_read_uint(cc_size n, cc_unit *r, size_t data_nbytes, const uint8_t *data);

/* r = (data, len) treated as a big endian byte array, return -1 if data
 doesn't fit in r, return 0 otherwise.
 ccn_read_uint strips leading zeroes and doesn't care about sign. */
#define ccn_read_int(n, r, data_size, data) ccn_read_uint(n, r, data_size, data)

/*!
 @function   ccn_write_uint_size
 @abstract   Compute the minimum size required to store an big integer

 @param n           Input size of the cc_unit array representing the input
 @param s           Input cc_unit array

 @result Return value is the exact byte size of the big integer

 @discussion
 The execution flow is independent on the value of the big integer.
 However, the use of the returned value may leak the position of the most significant byte
 */
CC_PURE CC_NONNULL((2)) size_t ccn_write_uint_size(cc_size n, const cc_unit *s);

/*!
 @function   ccn_write_uint
 @abstract   Serialize the big integer into a big endian byte buffer

 @param n           Input size of the cc_unit array representing the input
 @param s           Input cc_unit array
 @param out_size    Size of the output buffer
 @param out         Output byte array of size at least  out_size

 @discussion This function writes exactly
 MIN(out_size,ccn_write_uint_size(n,s)) bytes truncating to keep the
 most significant bytes when out_size<ccn_write_uint_size(n,s). The
 execution flow of function is based on the position of the most
 significant byte as well as input sizes.

 */

CC_NONNULL((2, 4))
void ccn_write_uint(cc_size n, const cc_unit *s, size_t out_size, void *out);

/*!
 @function   ccn_write_uint_padded_ct
 @abstract   Serialize the big integer into a big endian byte buffer

 @param n           Input size of the cc_unit array representing the input
 @param s           Input cc_unit array
 @param out_size    Size of the output buffer
 @param out         Output byte array of size at least  out_size

 @return number of leading zero bytes in case of success, a negative error value in case of failure

 @result  This function writes exactly out_size byte, padding with zeroes when necessary.
 This function DOES NOT support truncation and returns an error if out_size < ccn_write_uint_size

 @discussion The execution flow of function is independent on the value of the big integer
 However, the processing of the return value by the caller may expose the position of
 the most significant byte
 */
CC_NONNULL((2, 4))
int ccn_write_uint_padded_ct(cc_size n, const cc_unit *s, size_t out_size, uint8_t *out);

/*!
 @function   ccn_write_uint_padded
 @abstract   Serialize the big integer into a big endian byte buffer
 Not recommended, for most cases ccn_write_uint_padded_ct is more appropriate
 Sensitive big integers are exposed since the processing expose the position of the MS byte

 @param n           Input size of the cc_unit array representing the input
 @param s           Input cc_unit array
 @param out_size    Size of the output buffer
 @param out         Output byte array of size at least  out_size

 @return number of leading zero bytes

 @result  This function writes exactly out_size byte, padding with zeroes when necessary.
 This function DOES support truncation when out_size<ccn_write_uint_size()

 @discussion The execution flow of this function DEPENDS on the position of the most significant byte in
 case truncation is required.
 */

CC_INLINE CC_NONNULL((2, 4)) size_t ccn_write_uint_padded(cc_size n, const cc_unit *s, size_t out_size, uint8_t *out)
{
    size_t offset = 0;
    // Try first the non-truncation case
    int offset_int = ccn_write_uint_padded_ct(n, s, out_size, out);
    if (offset_int >= 0) {
        // It worked
        offset = (size_t)offset_int;
    } else {
        // Truncation case, execution depends on the position of the MSByte
        ccn_write_uint(n, s, out_size, out);
    }
    return offset;
}


/*  Return actual size in bytes needed to serialize s as int
    (adding leading zero if high bit is set). */
CC_PURE CC_NONNULL((2))
size_t ccn_write_int_size(cc_size n, const cc_unit *s);

/*  Serialize s, to out.
    First byte of byte stream is the m.s. byte of s,
    regardless of the size of cc_unit.

    No assumption is made about the alignment of out.

    The out_size argument should be the value returned from ccn_write_int_size,
    and is also the exact number of bytes this function will write to out.
    If out_size if less than the value returned by ccn_write_int_size, only the
    first out_size non-zero most significant octets of s will be written. */
CC_NONNULL((2, 4))
void ccn_write_int(cc_size n, const cc_unit *s, size_t out_size, void *out);

/* s -> r
 { n bit -> n bit } */
CC_NONNULL((2, 3))
void ccn_set(cc_size n, cc_unit *r, const cc_unit *s) __asm__("_ccn_set");

CC_INLINE CC_NONNULL((2))
void ccn_zero(cc_size n, cc_unit *r) {
    cc_clear(ccn_sizeof_n(n),r);
}

CC_INLINE CC_NONNULL((2))
void ccn_clear(cc_size n, cc_unit *r) {
    cc_clear(ccn_sizeof_n(n),r);
}

CC_NONNULL((2))
void ccn_zero_multi(cc_size n, cc_unit *r, ...);

CC_INLINE CC_NONNULL((2))
void ccn_seti(cc_size n, cc_unit *r, cc_unit v) {
    /* assert(n > 0); */
    r[0] = v;
    ccn_zero(n - 1, r + 1);
}

CC_INLINE CC_NONNULL((2, 4))
void ccn_setn(cc_size n, cc_unit *r, const cc_size s_size, const cc_unit *s) {
    /* FIXME: assert not available in kernel.
    assert(n > 0);
    assert(s_size > 0);
    assert(s_size <= n);
    */
    ccn_set(s_size, r, s);
    ccn_zero(n - s_size, r + s_size);
}

#define CC_SWAP_HOST_BIG_64(x) \
    ((uint64_t)((((uint64_t)(x) & 0xff00000000000000ULL) >> 56) | \
    (((uint64_t)(x) & 0x00ff000000000000ULL) >> 40) | \
    (((uint64_t)(x) & 0x0000ff0000000000ULL) >> 24) | \
    (((uint64_t)(x) & 0x000000ff00000000ULL) >>  8) | \
    (((uint64_t)(x) & 0x00000000ff000000ULL) <<  8) | \
    (((uint64_t)(x) & 0x0000000000ff0000ULL) << 24) | \
    (((uint64_t)(x) & 0x000000000000ff00ULL) << 40) | \
    (((uint64_t)(x) & 0x00000000000000ffULL) << 56)))
#define CC_SWAP_HOST_BIG_32(x) \
    ((((x) & 0xff000000) >> 24) | \
    (((x) & 0x00ff0000) >>  8) | \
    (((x) & 0x0000ff00) <<  8) | \
    (((x) & 0x000000ff) <<  24))
#define CC_SWAP_HOST_BIG_16(x) \
    ((((x) & 0xff00) >>  8) | \
    (((x) & 0x00ff) <<  8))

/* This should probably move if we move ccn_swap out of line. */
#if CCN_UNIT_SIZE == 8
#define CC_UNIT_TO_BIG(x) CC_SWAP_HOST_BIG_64(x)
#elif CCN_UNIT_SIZE == 4
#define CC_UNIT_TO_BIG(x) CC_SWAP_HOST_BIG_32(x)
#elif CCN_UNIT_SIZE == 2
#define CC_UNIT_TO_BIG(x) CC_SWAP_HOST_BIG_16(x)
#elif CCN_UNIT_SIZE == 1
#define CC_UNIT_TO_BIG(x) (x)
#else
#error unsupported CCN_UNIT_SIZE
#endif

/* Swap units in r in place from cc_unit vector byte order to big endian byte order (or back). */
CC_INLINE CC_NONNULL((2))
void ccn_swap(cc_size n, cc_unit *r) {
    cc_unit *e;
    for (e = r + n - 1; r < e; ++r, --e) {
        cc_unit t = CC_UNIT_TO_BIG(*r);
        *r = CC_UNIT_TO_BIG(*e);
        *e = t;
    }
    if (n & 1)
        *r = CC_UNIT_TO_BIG(*r);
}

CC_INLINE CC_NONNULL((2, 3, 4))
void ccn_xor(cc_size n, cc_unit *r, const cc_unit *s, const cc_unit *t) {
    while (n--) {
        r[n] = s[n] ^ t[n];
    }
}

/* Debugging */
CC_NONNULL((2))
void ccn_print(cc_size n, const cc_unit *s);
CC_NONNULL((3))
void ccn_lprint(cc_size n, const char *label, const cc_unit *s);

/* Forward declaration so we don't depend on ccrng.h. */
struct ccrng_state;

#if 0
CC_INLINE CC_NONNULL((2, 3))
int ccn_random(cc_size n, cc_unit *r, struct ccrng_state *rng) {
    return (RNG)->generate((RNG), ccn_sizeof_n(n), (unsigned char *)r);
}
#else
#define ccn_random(_n_,_r_,_ccrng_ctx_) \
    ccrng_generate(_ccrng_ctx_, ccn_sizeof_n(_n_), (unsigned char *)_r_)
#endif

/* Make a ccn of size ccn_nof(nbits) units with up to nbits sized random value. */
CC_NONNULL((2, 3))
int ccn_random_bits(cc_size nbits, cc_unit *r, struct ccrng_state *rng);

CC_NONNULL((6, 8))
int ccn_div_euclid(cc_size nq, cc_unit *q, cc_size nr, cc_unit *r, cc_size na, const cc_unit *a, cc_size nd, const cc_unit *d);

#define ccn_div(nq, q, na, a, nd, d) ccn_div_euclid(nq, q, 0, NULL, na, a, nd, d)
#define ccn_mod(nr, r, na, a, nd, d) ccn_div_euclid(0 , NULL, nr, r, na, a, nd, d)

#endif /* _CORECRYPTO_CCN_H_ */
