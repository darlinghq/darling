/*
 * Copyright (c) 1999-2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * "Portions Copyright (c) 1999 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 1992 NeXT Computer, Inc.
 *
 * Byte ordering conversion.
 *
 */

#ifndef	_ARCHITECTURE_BYTE_ORDER_H_
#define _ARCHITECTURE_BYTE_ORDER_H_
 
#include <libkern/OSByteOrder.h>

typedef unsigned long NXSwappedFloat;
typedef unsigned long long NXSwappedDouble;

static __inline__
unsigned short 
NXSwapShort(
    unsigned short inv
)
{
    return (unsigned short)OSSwapInt16((uint16_t)inv);
}

static __inline__
unsigned int
NXSwapInt(
    unsigned int inv
)
{
    return (unsigned int)OSSwapInt32((uint32_t)inv);
}

static __inline__
unsigned long
NXSwapLong(
    unsigned long inv
)
{
    return (unsigned long)OSSwapInt32((uint32_t)inv);
}

static __inline__
unsigned long long
NXSwapLongLong(
    unsigned long long inv
)
{
    return (unsigned long long)OSSwapInt64((uint64_t)inv);
}

static __inline__ NXSwappedFloat
NXConvertHostFloatToSwapped(float x)
{
    union fconv {
        float number;
        NXSwappedFloat sf;
    } u;
    u.number = x;
    return u.sf;
}

static __inline__ float
NXConvertSwappedFloatToHost(NXSwappedFloat x)
{
    union fconv {
        float number;
        NXSwappedFloat sf;
    } u;
    u.sf = x;
    return u.number;
}

static __inline__ NXSwappedDouble
NXConvertHostDoubleToSwapped(double x)
{
    union dconv {
        double number;
        NXSwappedDouble sd;
    } u;
    u.number = x;
    return u.sd;
}

static __inline__ double
NXConvertSwappedDoubleToHost(NXSwappedDouble x)
{
    union dconv {
        double number;
        NXSwappedDouble sd;
    } u;
    u.sd = x;
    return u.number;
}

static __inline__ NXSwappedFloat
NXSwapFloat(NXSwappedFloat x)
{ 
    return (NXSwappedFloat)OSSwapInt32((uint32_t)x);  
}

static __inline__ NXSwappedDouble   
NXSwapDouble(NXSwappedDouble x)
{  
    return (NXSwappedDouble)OSSwapInt64((uint64_t)x);
}

/*
 * Identify the byte order
 * of the current host.
 */

enum NXByteOrder {
    NX_UnknownByteOrder,
    NX_LittleEndian,
    NX_BigEndian
};

static __inline__
enum NXByteOrder
NXHostByteOrder(void)
{
#if defined(__LITTLE_ENDIAN__)
    return NX_LittleEndian;
#elif defined(__BIG_ENDIAN__)
    return NX_BigEndian;
#else
    return NX_UnknownByteOrder;
#endif
}

static __inline__
unsigned short
NXSwapBigShortToHost(
    unsigned short	x
)
{
    return (unsigned short)OSSwapBigToHostInt16((uint16_t)x);
}

static __inline__
unsigned int
NXSwapBigIntToHost(
    unsigned int	x
)
{
    return (unsigned int)OSSwapBigToHostInt32((uint32_t)x);
}

static __inline__
unsigned long
NXSwapBigLongToHost(
    unsigned long	x
)
{
    return (unsigned long)OSSwapBigToHostInt32((uint32_t)x);
}

static __inline__
unsigned long long
NXSwapBigLongLongToHost(
    unsigned long long	x
)
{
    return (unsigned long long)OSSwapBigToHostInt64((uint64_t)x);
}

static __inline__
double
NXSwapBigDoubleToHost(
    NXSwappedDouble	x
)
{
    return NXConvertSwappedDoubleToHost((NXSwappedDouble)OSSwapBigToHostInt64((uint64_t)x));
}

static __inline__
float
NXSwapBigFloatToHost(
    NXSwappedFloat	x
)
{
    return NXConvertSwappedFloatToHost((NXSwappedFloat)OSSwapBigToHostInt32((uint32_t)x));
}

static __inline__
unsigned short
NXSwapHostShortToBig(
    unsigned short	x
)
{
    return (unsigned short)OSSwapHostToBigInt16((uint16_t)x);
}

static __inline__
unsigned int
NXSwapHostIntToBig(
    unsigned int	x
)
{
    return (unsigned int)OSSwapHostToBigInt32((uint32_t)x);
}

static __inline__
unsigned long
NXSwapHostLongToBig(
    unsigned long	x
)
{
    return (unsigned long)OSSwapHostToBigInt32((uint32_t)x);
}

static __inline__
unsigned long long
NXSwapHostLongLongToBig(
    unsigned long long	x
)
{
    return (unsigned long long)OSSwapHostToBigInt64((uint64_t)x);
}

static __inline__
NXSwappedDouble
NXSwapHostDoubleToBig(
    double	x
)
{
    return (NXSwappedDouble)OSSwapHostToBigInt64((uint64_t)NXConvertHostDoubleToSwapped(x));
}

static __inline__
NXSwappedFloat
NXSwapHostFloatToBig(
    float	x
)
{
    return (NXSwappedFloat)OSSwapHostToBigInt32((uint32_t)NXConvertHostFloatToSwapped(x));
}

static __inline__
unsigned short
NXSwapLittleShortToHost(
    unsigned short	x
)
{
    return (unsigned short)OSSwapLittleToHostInt16((uint16_t)x);
}

static __inline__
unsigned int
NXSwapLittleIntToHost(
    unsigned int	x
)
{
    return (unsigned int)OSSwapLittleToHostInt32((uint32_t)x);
}

static __inline__
unsigned long
NXSwapLittleLongToHost(
    unsigned long	x
)
{
    return (unsigned long)OSSwapLittleToHostInt32((uint32_t)x);
}

static __inline__
unsigned long long
NXSwapLittleLongLongToHost(
    unsigned long long	x
)
{
    return (unsigned long long)OSSwapLittleToHostInt64((uint64_t)x);
}

static __inline__
double
NXSwapLittleDoubleToHost(
    NXSwappedDouble	x
)
{
    return NXConvertSwappedDoubleToHost((NXSwappedDouble)OSSwapLittleToHostInt64((uint64_t)x));
}

static __inline__
float
NXSwapLittleFloatToHost(
    NXSwappedFloat	x
)
{
    return NXConvertSwappedFloatToHost((NXSwappedFloat)OSSwapLittleToHostInt32((uint32_t)x));
}

static __inline__
unsigned short
NXSwapHostShortToLittle(
    unsigned short	x
)
{
    return (unsigned short)OSSwapHostToLittleInt16((uint16_t)x);
}

static __inline__
unsigned int
NXSwapHostIntToLittle(
    unsigned int	x
)
{
    return (unsigned int)OSSwapHostToLittleInt32((uint32_t)x);
}

static __inline__
unsigned long
NXSwapHostLongToLittle(
    unsigned long	x
)
{
    return (unsigned long)OSSwapHostToLittleInt32((uint32_t)x);
}

static __inline__
unsigned long long
NXSwapHostLongLongToLittle(
    unsigned long long	x
)
{
    return (unsigned long long)OSSwapHostToLittleInt64((uint64_t)x);
}

static __inline__
NXSwappedDouble
NXSwapHostDoubleToLittle(
    double	x
)
{
    return (NXSwappedDouble)OSSwapHostToLittleInt64((uint64_t)NXConvertHostDoubleToSwapped(x));
}

static __inline__
NXSwappedFloat
NXSwapHostFloatToLittle(
    float	x
)
{
    return (NXSwappedFloat)OSSwapHostToLittleInt32((uint32_t)NXConvertHostFloatToSwapped(x));
}

#endif	/* _ARCHITECTURE_BYTE_ORDER_H_ */
