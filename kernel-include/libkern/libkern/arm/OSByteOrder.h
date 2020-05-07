/*
 * Copyright (c) 1999-2007 Apple Inc. All rights reserved.
 */

#ifndef _OS_OSBYTEORDERARM_H
#define _OS_OSBYTEORDERARM_H

#include <stdint.h>
#include <arm/arch.h> /* for _ARM_ARCH_6 */
#include <sys/_types/_os_inline.h>

/* Generic byte swapping functions. */

OS_INLINE
uint16_t
_OSSwapInt16(
	uint16_t        data
	)
{
	/* Reduces to 'rev16' with clang */
	return (uint16_t)(data << 8 | data >> 8);
}

OS_INLINE
uint32_t
_OSSwapInt32(
	uint32_t        data
	)
{
#if defined(__llvm__)
	data = __builtin_bswap32(data);
#else
	/* This actually generates the best code */
	data = (((data ^ (data >> 16 | (data << 16))) & 0xFF00FFFF) >> 8) ^ (data >> 8 | data << 24);
#endif

	return data;
}

OS_INLINE
uint64_t
_OSSwapInt64(
	uint64_t        data
	)
{
#if defined(__llvm__)
	return __builtin_bswap64(data);
#else
	union {
		uint64_t ull;
		uint32_t ul[2];
	} u;

	/* This actually generates the best code */
	u.ul[0] = (uint32_t)(data >> 32);
	u.ul[1] = (uint32_t)(data & 0xffffffff);
	u.ul[0] = _OSSwapInt32(u.ul[0]);
	u.ul[1] = _OSSwapInt32(u.ul[1]);
	return u.ull;
#endif
}

/* Functions for byte reversed loads. */

OS_INLINE
uint16_t
OSReadSwapInt16(
	const volatile void   * base,
	uintptr_t       offset
	)
{
	uint16_t result;

	result = *(volatile uint16_t *)((volatile uintptr_t)base + offset);
	return _OSSwapInt16(result);
}

OS_INLINE
uint32_t
OSReadSwapInt32(
	const volatile void   * base,
	uintptr_t       offset
	)
{
	uint32_t result;

	result = *(volatile uint32_t *)((volatile uintptr_t)base + offset);
	return _OSSwapInt32(result);
}

OS_INLINE
uint64_t
OSReadSwapInt64(
	const volatile void   * base,
	uintptr_t       offset
	)
{
	volatile uint32_t * inp;
	union ullc {
		uint64_t     ull;
		uint32_t     ul[2];
	} outv;

	inp = (volatile uint32_t *)((volatile uintptr_t)base + offset);
	outv.ul[0] = inp[1];
	outv.ul[1] = inp[0];
	outv.ul[0] = _OSSwapInt32(outv.ul[0]);
	outv.ul[1] = _OSSwapInt32(outv.ul[1]);
	return outv.ull;
}

/* Functions for byte reversed stores. */

OS_INLINE
void
OSWriteSwapInt16(
	volatile void   * base,
	uintptr_t       offset,
	uint16_t        data
	)
{
	*(volatile uint16_t *)((volatile uintptr_t)base + offset) = _OSSwapInt16(data);
}

OS_INLINE
void
OSWriteSwapInt32(
	volatile void   * base,
	uintptr_t       offset,
	uint32_t        data
	)
{
	*(volatile uint32_t *)((volatile uintptr_t)base + offset) = _OSSwapInt32(data);
}

OS_INLINE
void
OSWriteSwapInt64(
	volatile void    * base,
	uintptr_t        offset,
	uint64_t         data
	)
{
	*(volatile uint64_t *)((volatile uintptr_t)base + offset) = _OSSwapInt64(data);
}

#endif /* ! _OS_OSBYTEORDERARM_H */
