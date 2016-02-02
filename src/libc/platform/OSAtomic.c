#include <libkern/OSAtomic.h>

// In OS X 10.10 (or something) the following became full-fledged symbols
// provided in libsystem_platform (not released as open source).

int32_t	OSAtomicIncrement32( volatile int32_t *__theValue )
            { return OSAtomicAdd32(  1, __theValue); }


int32_t	OSAtomicIncrement32Barrier( volatile int32_t *__theValue )
            { return OSAtomicAdd32Barrier(  1, __theValue); }

int32_t	OSAtomicDecrement32( volatile int32_t *__theValue )
            { return OSAtomicAdd32( -1, __theValue); }

int32_t	OSAtomicDecrement32Barrier( volatile int32_t *__theValue )
            { return OSAtomicAdd32Barrier( -1, __theValue); }


int64_t	OSAtomicIncrement64( volatile int64_t *__theValue )
            { return OSAtomicAdd64(  1, __theValue); }

int64_t	OSAtomicIncrement64Barrier( volatile int64_t *__theValue )
            { return OSAtomicAdd64Barrier(  1, __theValue); }


int64_t	OSAtomicDecrement64( volatile int64_t *__theValue )
            { return OSAtomicAdd64( -1, __theValue); }


int64_t	OSAtomicDecrement64Barrier( volatile int64_t *__theValue )
            { return OSAtomicAdd64Barrier( -1, __theValue); }

