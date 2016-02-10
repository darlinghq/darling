/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 */
/*
 * Copyright (c) 1999 Apple Computer, Inc.  All rights reserved.
 *
 * HISTORY
 *
 */

#ifndef _OS_OSATOMIC_H
#define _OS_OSATOMIC_H

#include <libkern/OSBase.h>

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef XNU_KERNEL_PRIVATE
/*
 * The macro SAFE_CAST_PTR() casts one type of pointer to another type, making sure
 * the data the pointer is referencing is the same size. If it is not, it will cause
 * a division by zero compiler warning. This is to work around "SInt32" being defined
 * as "long" on ILP32 and as "int" on LP64, which would require an explicit cast to
 * "SInt32*" when for instance passing an "int*" to OSAddAtomic() - which masks size
 * mismatches.
 * -- var is used, but sizeof does not evaluate the
 *    argument, i.e. we're safe against "++" etc. in var --
 */
#define __SAFE_CAST_PTR(type, var) (((type)(var))+(0/(sizeof(*var) == sizeof(*(type)0) ? 1 : 0)))
#else
#define __SAFE_CAST_PTR(type, var) ((type)(var))
#endif

/*!
 * @header
 *
 * @abstract
 * This header declares the OSAtomic group of functions for atomic
 * reading and updating of values.
 */
 
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__)

/*!
 * @function OSCompareAndSwap64
 *
 * @abstract
 * 64-bit compare and swap operation.
 *
 * @discussion
 * See OSCompareAndSwap.
 */
extern Boolean OSCompareAndSwap64(
    UInt64            oldValue,
    UInt64            newValue,
    volatile UInt64 * address);
#define OSCompareAndSwap64(a, b, c) \
	(OSCompareAndSwap64(a, b, __SAFE_CAST_PTR(volatile UInt64*,c)))

#endif /* defined(__i386__) || defined(__x86_64__) */

/*!
 * @function OSAddAtomic64
 *
 * @abstract
 * 64-bit atomic add operation.
 *
 * @discussion
 * See OSAddAtomic.
 */
extern SInt64 OSAddAtomic64(
    SInt64            theAmount,
    volatile SInt64 * address);
#define OSAddAtomic64(a, b) \
	(OSAddAtomic64(a, __SAFE_CAST_PTR(volatile SInt64*,b)))

/*!
 * @function OSIncrementAtomic64
 *
 * @abstract
 * 64-bit increment.
 *
 * @discussion
 * See OSIncrementAtomic.
*/
inline static SInt64 OSIncrementAtomic64(volatile SInt64 * address)
{
    return OSAddAtomic64(1LL, address);
}

/*!
 * @function OSDecrementAtomic64
 *
 * @abstract
 * 64-bit decrement.
 *
 * @discussion
 * See OSDecrementAtomic.
*/
inline static SInt64 OSDecrementAtomic64(volatile SInt64 * address)
{
    return OSAddAtomic64(-1LL, address);
}

#if XNU_KERNEL_PRIVATE
/* Not to be included in headerdoc.
 *
 * @function OSAddAtomicLong
 *
 * @abstract
 * 32/64-bit atomic add operation, depending on sizeof(long).
 *
 * @discussion
 * See OSAddAtomic.
 */
extern long OSAddAtomicLong(
    long            theAmount,
    volatile long * address);
#define OSAddAtomicLong(a, b) \
	(OSAddAtomicLong(a, __SAFE_CAST_PTR(volatile long*,b)))

/* Not to be included in headerdoc.
 *
 * @function OSIncrementAtomicLong
 *
 * @abstract
 * 32/64-bit increment, depending on sizeof(long)
 *
 * @discussion
 * See OSIncrementAtomic.
*/
inline static long OSIncrementAtomicLong(volatile long * address)
{
    return OSAddAtomicLong(1L, address);
}

/* Not to be included in headerdoc.
 *
 * @function OSDecrementAtomicLong
 *
 * @abstract
 * 32/64-bit decrement, depending on sizeof(long)
 *@discussion See OSDecrementAtomic.
 */
inline static long OSDecrementAtomicLong(volatile long * address)
{
    return OSAddAtomicLong(-1L, address);
}
#endif /* XNU_KERNEL_PRIVATE */

/*!
 * @function OSCompareAndSwap
 *
 * @abstract
 * Compare and swap operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSCompareAndSwap function compares the value at the specified address with oldVal. The value of newValue is written to the address only if oldValue and the value at the address are equal. OSCompareAndSwap returns true if newValue is written to the address; otherwise, it returns false.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 *
 * @param oldValue The value to compare at address.
 * @param newValue The value to write to address if oldValue compares true.
 * @param address The 4-byte aligned address of the data to update atomically.
 * @result true if newValue was written to the address.
 */
extern Boolean OSCompareAndSwap(
    UInt32            oldValue,
    UInt32            newValue,
    volatile UInt32 * address);
#define OSCompareAndSwap(a, b, c) \
	(OSCompareAndSwap(a, b, __SAFE_CAST_PTR(volatile UInt32*,c)))

/*!
 * @function OSCompareAndSwapPtr
 *
 * @abstract
 * Compare and swap operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSCompareAndSwapPtr function compares the pointer-sized value at the specified address with oldVal. The value of newValue is written to the address only if oldValue and the value at the address are equal. OSCompareAndSwapPtr returns true if newValue is written to the address; otherwise, it returns false.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param oldValue The pointer value to compare at address.
 * @param newValue The pointer value to write to address if oldValue compares true.
 * @param address The pointer-size aligned address of the data to update atomically.
 * @result true if newValue was written to the address.
 */
extern Boolean OSCompareAndSwapPtr(
    void            * oldValue,
    void            * newValue,
    void * volatile * address);
#define OSCompareAndSwapPtr(a, b, c) \
	(OSCompareAndSwapPtr(a, b, __SAFE_CAST_PTR(void * volatile *,c)))

/*!
 * @function OSAddAtomic
 *
 * @abstract
 * 32-bit add operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSAddAtomic function adds the specified amount to the value at the specified address and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param amount The amount to add.
 * @param address The 4-byte aligned address of the value to update atomically.
 * @result The value before the addition
 */
extern SInt32 OSAddAtomic(
    SInt32            amount,
    volatile SInt32 * address);
#define OSAddAtomic(a, b) \
	(OSAddAtomic(a, __SAFE_CAST_PTR(volatile SInt32*,b)))

/*!
 * @function OSAddAtomic16
 *
 * @abstract
 * 16-bit add operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSAddAtomic16 function adds the specified amount to the value at the specified address and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param amount The amount to add.
 * @param address The 2-byte aligned address of the value to update atomically.
 * @result The value before the addition
 */
extern SInt16 OSAddAtomic16(
    SInt32            amount,
    volatile SInt16 * address);

/*!
 * @function OSAddAtomic8
 *
 * @abstract
 * 8-bit add operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSAddAtomic8 function adds the specified amount to the value at the specified address and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param amount The amount to add.
 * @param address The address of the value to update atomically.
 * @result The value before the addition.
 */
extern SInt8 OSAddAtomic8(
    SInt32           amount,
    volatile SInt8 * address);

/*!
 * @function OSIncrementAtomic
 *
 * @abstract
 * 32-bit increment operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSIncrementAtomic function increments the value at the specified address by one and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device.
 * @param address The 4-byte aligned address of the value to update atomically.
 * @result The value before the increment.
 */
extern SInt32 OSIncrementAtomic(volatile SInt32 * address);
#define OSIncrementAtomic(a) \
	(OSIncrementAtomic(__SAFE_CAST_PTR(volatile SInt32*,a)))

/*!
 * @function OSIncrementAtomic16
 *
 * @abstract
 * 16-bit increment operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSIncrementAtomic16 function increments the value at the specified address by one and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param address The 2-byte aligned address of the value to update atomically.
 * @result The value before the increment.
 */
extern SInt16 OSIncrementAtomic16(volatile SInt16 * address);

/*!
 * @function OSIncrementAtomic8
 *
 * @abstract
 * 8-bit increment operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSIncrementAtomic8 function increments the value at the specified address by one and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param address The address of the value to update atomically.
 * @result The value before the increment.
 */
extern SInt8 OSIncrementAtomic8(volatile SInt8 * address);

/*!
 * @function OSDecrementAtomic
 *
 * @abstract
 * 32-bit decrement operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSDecrementAtomic function decrements the value at the specified address by one and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param address The 4-byte aligned address of the value to update atomically.
 * @result The value before the decrement.
 */
extern SInt32 OSDecrementAtomic(volatile SInt32 * address);
#define OSDecrementAtomic(a) \
	(OSDecrementAtomic(__SAFE_CAST_PTR(volatile SInt32*,a)))

/*!
 * @function OSDecrementAtomic16
 *
 * @abstract
 * 16-bit decrement operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSDecrementAtomic16 function decrements the value at the specified address by one and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param address The 2-byte aligned address of the value to update atomically.
 * @result The value before the decrement.
 */
extern SInt16 OSDecrementAtomic16(volatile SInt16 * address);

/*!
 * @function OSDecrementAtomic8
 *
 * @abstract
 * 8-bit decrement operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSDecrementAtomic8 function decrements the value at the specified address by one and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param address The address of the value to update atomically.
 * @result The value before the decrement.
 */
extern SInt8 OSDecrementAtomic8(volatile SInt8 * address);

/*!
 * @function OSBitAndAtomic
 *
 * @abstract
 * 32-bit logical and operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSBitAndAtomic function logically ands the bits of the specified mask into the value at the specified address and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param mask The mask to logically and with the value.
 * @param address The 4-byte aligned address of the value to update atomically.
 * @result The value before the bitwise operation
 */
extern UInt32 OSBitAndAtomic(
    UInt32            mask,
    volatile UInt32 * address);
#define OSBitAndAtomic(a, b) \
	(OSBitAndAtomic(a, __SAFE_CAST_PTR(volatile UInt32*,b)))

/*!
 * @function OSBitAndAtomic16
 *
 * @abstract
 * 16-bit logical and operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSBitAndAtomic16 function logically ands the bits of the specified mask into the value at the specified address and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param mask The mask to logically and with the value.
 * @param address The 2-byte aligned address of the value to update atomically.
 * @result The value before the bitwise operation.
 */
extern UInt16 OSBitAndAtomic16(
    UInt32            mask,
    volatile UInt16 * address);

/*!
 * @function OSBitAndAtomic8
 *
 * @abstract
 * 8-bit logical and operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSBitAndAtomic8 function logically ands the bits of the specified mask into the value at the specified address and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param mask The mask to logically and with the value.
 * @param address The address of the value to update atomically.
 * @result The value before the bitwise operation.
 */
extern UInt8 OSBitAndAtomic8(
    UInt32           mask,
    volatile UInt8 * address);

/*!
 * @function OSBitOrAtomic
 *
 * @abstract
 * 32-bit logical or operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSBitOrAtomic function logically ors the bits of the specified mask into the value at the specified address and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param mask The mask to logically or with the value.
 * @param address The 4-byte aligned address of the value to update atomically.
 * @result The value before the bitwise operation.
 */
extern UInt32 OSBitOrAtomic(
    UInt32            mask,
    volatile UInt32 * address);
#define OSBitOrAtomic(a, b) \
	(OSBitOrAtomic(a, __SAFE_CAST_PTR(volatile UInt32*,b)))

/*!
 * @function OSBitOrAtomic16
 *
 * @abstract
 * 16-bit logical or operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSBitOrAtomic16 function logically ors the bits of the specified mask into the value at the specified address and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param mask The mask to logically or with the value.
 * @param address The 2-byte aligned address of the value to update atomically.
 * @result The value before the bitwise operation.
 */
extern UInt16 OSBitOrAtomic16(
    UInt32            mask,
    volatile UInt16 * address);

/*!
 * @function OSBitOrAtomic8
 *
 * @abstract
 * 8-bit logical or operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 *
 * @discussion
 * The OSBitOrAtomic8 function logically ors the bits of the specified mask into the value at the specified address and returns the original value.
 * @param mask The mask to logically or with the value.
 * @param address The address of the value to update atomically.
 * @result The value before the bitwise operation.
 */
extern UInt8 OSBitOrAtomic8(
    UInt32           mask,
    volatile UInt8 * address);

/*!
 * @function OSBitXorAtomic
 *
 * @abstract
 * 32-bit logical xor operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 *
 * @discussion
 * The OSBitXorAtomic function logically xors the bits of the specified mask into the value at the specified address and returns the original value.
 * @param mask The mask to logically or with the value.
 * @param address The 4-byte aligned address of the value to update atomically.
 * @result The value before the bitwise operation.
 */
extern UInt32 OSBitXorAtomic(
    UInt32            mask,
    volatile UInt32 * address);
#define OSBitXorAtomic(a, b) \
	(OSBitXorAtomic(a, __SAFE_CAST_PTR(volatile UInt32*,b)))

/*!
 * @function OSBitXorAtomic16
 *
 * @abstract
 * 16-bit logical xor operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSBitXorAtomic16 function logically xors the bits of the specified mask into the value at the specified address and returns the original value.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param mask The mask to logically or with the value.
 * @param address The 2-byte aligned address of the value to update atomically.
 * @result The value before the bitwise operation.
 */
extern UInt16 OSBitXorAtomic16(
    UInt32            mask,
    volatile UInt16 * address);

/*!
 * @function OSBitXorAtomic8
 *
 * @abstract
 * 8-bit logical xor operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 *
 * @discussion
 * The OSBitXorAtomic8 function logically xors the bits of the specified mask into the value at the specified address and returns the original value.
 * @param mask The mask to logically or with the value.
 * @param address The address of the value to update atomically.
 * @result The value before the bitwise operation.
 */
extern UInt8 OSBitXorAtomic8(
    UInt32           mask,
    volatile UInt8 * address);

/*!
 * @function OSTestAndSet
 *
 * @abstract
 * Bit test and set operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 *
 * @discussion
 * The OSTestAndSet function sets a single bit in a byte at a specified address. It returns true if the bit was already set, false otherwise.
 * @param bit The bit number in the range 0 through 7.
 * @param startAddress The address of the byte to update atomically.
 * @result true if the bit was already set, false otherwise.
 */
extern Boolean OSTestAndSet(
    UInt32           bit,
    volatile UInt8 * startAddress);

/*!
 * @function OSTestAndClear
 *
 * @abstract
 * Bit test and clear operation, performed atomically with respect to all devices that participate in the coherency architecture of the platform.
 *
 * @discussion
 * The OSTestAndClear function clears a single bit in a byte at a specified address. It returns true if the bit was already clear, false otherwise.
 *
 * This function guarantees atomicity only with main system memory. It is specifically unsuitable for use on noncacheable memory such as that in devices; this function cannot guarantee atomicity, for example, on memory mapped from a PCI device. Additionally, this function incorporates a memory barrier on systems with weakly-ordered memory architectures.
 * @param bit The bit number in the range 0 through 7.
 * @param startAddress The address of the byte to update atomically.
 * @result true if the bit was already clear, false otherwise.
 */
extern Boolean OSTestAndClear(
    UInt32           bit,
    volatile UInt8 * startAddress);

/*!
 * @defined OS_SPINLOCK_INIT
 *
 * @abstract
 * The default value for an OSSpinLock.
 *
 * @discussion
 * The convention is that unlocked is zero, locked is nonzero.
 */
#define	OS_SPINLOCK_INIT 0

/*! 
 * @typedef OSSpinLock
 *
 * @abstract
 * Data type for a spinlock.
 *
 * @discussion
 * You should always initialize a spinlock to OS_SPINLOCK_INIT before using it.
 */
typedef SInt32 OSSpinLock;

#ifdef PRIVATE
/*!
 * @function OSSpinLockTry
 *
 * @abstract
 * Locks a spinlock if it would not block.
 *
 * @discussion
 * Multiprocessor locks used within the shared memory area between the kernel and event system.  These must work in both user and kernel mode.
 *
 * @result
 * Returns false if the lock was already held by another thread, true if it took the lock successfully. 
 */
extern Boolean OSSpinLockTry(volatile OSSpinLock * lock);

/*!
 * @function OSSpinLockUnlock
 *
 * @abstract
 * Unlocks a spinlock.
 *
 * @discussion
 * Unlocks a spinlock.
 */
extern void OSSpinLockUnlock(volatile OSSpinLock * lock);
#endif /* PRIVATE */

/*!
 * @function OSSynchronizeIO
 *
 * @abstract
 * The OSSynchronizeIO routine ensures orderly load and store operations to noncached memory mapped I/O devices.
 *
 * @discussion
 * The OSSynchronizeIO routine ensures orderly load and store operations to noncached memory mapped I/O devices. It executes the eieio instruction on PowerPC processors.
 */
static __inline__ void OSSynchronizeIO(void)
{
}
#if	defined(XNU_KERNEL_PRIVATE)
#if   defined(__i386__) || defined(__x86_64__)
static inline void OSMemoryBarrier(void) {
	__asm__ volatile("mfence" ::: "memory");
}
#endif
#endif /*XNU_KERNEL_PRIVATE */
#if defined(__cplusplus)
}
#endif

#endif /* ! _OS_OSATOMIC_H */
