#ifndef _OS__OSBYTEORDERARM_H
#define _OS__OSBYTEORDERARM_H

#if !defined(__DARWIN_OS_INLINE)
# if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#        define __DARWIN_OS_INLINE static inline
# elif defined(__MWERKS__) || defined(__cplusplus)
#        define __DARWIN_OS_INLINE static inline
# else
#        define __DARWIN_OS_INLINE static __inline__
# endif
#endif

/* Generic byte swapping functions. */

__DARWIN_OS_INLINE
__uint16_t
_OSSwapInt16(
	__uint16_t        _data
	)
{
	return (__uint16_t)((_data << 8) | (_data >> 8));
}

__DARWIN_OS_INLINE
__uint32_t
_OSSwapInt32(
	__uint32_t        _data
	)
{
	return __builtin_bswap32(_data);
}

__DARWIN_OS_INLINE
__uint64_t
_OSSwapInt64(
	__uint64_t        _data
	)
{
	return __builtin_bswap64(_data);
}

#endif /* ! _OS__OSBYTEORDERARM_H */
