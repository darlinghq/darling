#ifndef _OS_OSBYTEORDERARM_H
#define _OS_OSBYTEORDERARM_H

#include <stdint.h>
#include <libkern/arm/_OSByteOrder.h>
#include <sys/_types/_os_inline.h>

/* Functions for byte reversed loads. */

OS_INLINE
uint16_t
OSReadSwapInt16(
	const volatile void   * base,
	uintptr_t       byteOffset
	)
{
	uint16_t result;
	result = *(volatile uint16_t *)((uintptr_t)base + byteOffset);
	return _OSSwapInt16(result);
}

OS_INLINE
uint32_t
OSReadSwapInt32(
	const volatile void   * base,
	uintptr_t       byteOffset
	)
{
	uint32_t result;
	result = *(volatile uint32_t *)((uintptr_t)base + byteOffset);
	return _OSSwapInt32(result);
}

OS_INLINE
uint64_t
OSReadSwapInt64(
	const volatile void   * base,
	uintptr_t       byteOffset
	)
{
	uint64_t result;
	result = *(volatile uint64_t *)((uintptr_t)base + byteOffset);
	return _OSSwapInt64(result);
}

/* Functions for byte reversed stores. */

OS_INLINE
void
OSWriteSwapInt16(
	volatile void   * base,
	uintptr_t       byteOffset,
	uint16_t        data
	)
{
	*(volatile uint16_t *)((uintptr_t)base + byteOffset) = _OSSwapInt16(data);
}

OS_INLINE
void
OSWriteSwapInt32(
	volatile void   * base,
	uintptr_t       byteOffset,
	uint32_t        data
	)
{
	*(volatile uint32_t *)((uintptr_t)base + byteOffset) = _OSSwapInt32(data);
}

OS_INLINE
void
OSWriteSwapInt64(
	volatile void    * base,
	uintptr_t        byteOffset,
	uint64_t         data
	)
{
	*(volatile uint64_t *)((uintptr_t)base + byteOffset) = _OSSwapInt64(data);
}

#endif /* ! _OS_OSBYTEORDERARM_H */
