/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 *
 * HISTORY
 *   gvdl    20050620    Created
 */

// xx-review: are these even used anywhere? Grep turns up squat.


/*!
 * @header OSEndianTypes
 *
 * @abstract
 * C++ inline types for byte-swapping.
 *
 * @discussion
 * The OSEndianTypes consist of a number of types that are used
 * very similarly to the traditional MacOS C scalar integers types,
 * for example, <code>UInt32</code> and <code>SInt32</code>.
 * @copyright 2005 Apple Computer, Inc. All rights reserved.
 * @updated 2005-07-25
 */

// Header doc magic trick for simple documentation
#if 0
/*!
 * @typedef BigUInt16
 * @abstract A Big-endian unsigned integer scalar size 16 - UInt16
 */
typedef class BigUInt16 BigUInt16;

/*!
 * @typedef BigSInt16
 * @abstract A Big-endian signed integer scalar size 16 - SInt16
 */
typedef class BigSInt16 BigSInt16;

/*!
 * @typedef BigUInt32
 * @abstract A Big-endian unsigned integer scalar size 32 - UInt32
 */
typedef class BigUInt32 BigUInt32;

/*!
 * @typedef BigSInt32
 * @abstract A Big-endian signed integer scalar size 32 - SInt32
 */
typedef class BigSInt32 BigSInt32;

/*!
 * @typedef BigUInt64
 * @abstract A Big-endian unsigned integer scalar size 64 - UInt64
 */
typedef class BigUInt64 BigUInt64;

/*!
 * @typedef BigSInt64
 * @abstract A Big-endian signed integer scalar size 64 - SInt64
 */
typedef class BigSInt64 BigSInt64;

/*!
 * @typedef LittleUInt16
 * @abstract A Little-endian unsigned integer scalar size 16 - UInt16
 */
typedef class LittleUInt16 LittleUInt16;

/*!
 * @typedef LittleSInt16
 * @abstract A Little-endian signed integer scalar size 16 - SInt16
 */
typedef class LittleSInt16 LittleSInt16;

/*!
 * @typedef LittleUInt32
 * @abstract A Little-endian unsigned integer scalar size 32 - UInt32
 */
typedef class LittleUInt32 LittleUInt32;

/*!
 * @typedef LittleSInt32
 * @abstract A Little-endian signed integer scalar size 32 - SInt32
 */
typedef class LittleSInt32 LittleSInt32;

/*!
 * @typedef LittleUInt64
 * @abstract A Little-endian unsigned integer scalar size 64 - UInt64
 */
typedef class LittleUInt64 LittleUInt64;

/*!
 * @typedef LittleSInt64
 * @abstract A Little-endian signed integer scalar size 64 - SInt64
 */
typedef class LittleSInt64 LittleSInt64;

#endif /* 0 - headerdoc trick */

#ifndef _OS_OSENDIANHELPER_H
#define _OS_OSENDIANHELPER_H

#if __cplusplus

#include <libkern/OSTypes.h>
#include <libkern/OSByteOrder.h>

// Probably should really be using templates, this is one of the few cases
// where they do make sense.  But as the kernel is not allowed to export
// template based C++ APIs we have to use sophisticated macros instead
#define __OSEndianSignIntSizeDEF(argname, argend, argtype, argsize) {  \
public:                                                                \
    typedef argtype ## argsize        Value;                           \
                                                                       \
private:                                                               \
    typedef UInt ## argsize        UValue;                             \
    UValue mValue;                                                     \
                                                                       \
    void writeValue(Value v) {                                         \
    if (__builtin_constant_p(v))                                       \
	mValue = OSSwapHostTo ## argend ## ConstInt ## argsize(v);     \
    else                                                               \
	OSWrite ## argend ## Int ## argsize(&mValue, 0, (UValue) v);   \
    };                                                                 \
                                                                       \
    Value readValue() const {                                          \
    return (Value) OSRead ## argend ## Int ## argsize(&mValue, 0);     \
    };                                                                 \
                                                                       \
public:                                                                \
    argname() { };                                                     \
                                                                       \
    argname (Value v) { writeValue(v); };                              \
    argname  &operator = (Value v) { writeValue(v); return *this; }    \
                                                                       \
    Value get() const { return readValue(); };                         \
    operator Value () const { return readValue(); };                   \
}

class BigUInt16    __OSEndianSignIntSizeDEF(BigUInt16, Big, UInt, 16);
class BigSInt16    __OSEndianSignIntSizeDEF(BigSInt16, Big, SInt, 16);
class BigUInt32    __OSEndianSignIntSizeDEF(BigUInt32, Big, UInt, 32);
class BigSInt32    __OSEndianSignIntSizeDEF(BigSInt32, Big, SInt, 32);
class BigUInt64    __OSEndianSignIntSizeDEF(BigUInt64, Big, UInt, 64);
class BigSInt64    __OSEndianSignIntSizeDEF(BigSInt64, Big, SInt, 64);
class LittleUInt16 __OSEndianSignIntSizeDEF(LittleUInt16, Little, UInt, 16);
class LittleSInt16 __OSEndianSignIntSizeDEF(LittleSInt16, Little, SInt, 16);
class LittleUInt32 __OSEndianSignIntSizeDEF(LittleUInt32, Little, UInt, 32);
class LittleSInt32 __OSEndianSignIntSizeDEF(LittleSInt32, Little, SInt, 32);
class LittleUInt64 __OSEndianSignIntSizeDEF(LittleUInt64, Little, UInt, 64);
class LittleSInt64 __OSEndianSignIntSizeDEF(LittleSInt64, Little, SInt, 64);

#undef __OSEndianSignIntSizeDEF

#endif /* __cplusplus
        */

#endif /* ! _OS_OSENDIANHELPER_H
        */
