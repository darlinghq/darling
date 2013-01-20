#ifndef CGERROR_H
#define CGERROR_H
#include <stdint.h>

typedef int32_t CGError;

enum : CGError
{
	kCGErrorSuccess = 0,
	kCGErrorFailure = 1000,
	kCGErrorIllegalArgument,
	kCGErrorInvalidConnection,
	kCGErrorInvalidContext,
	kCGErrorCannotComplete,
	kCGErrorNotImplemented,
	kCGErrorRangeCheck,
	kCGErrorTypeCheck,
	kCGErrorInvalidOperation = 1010,
	kCGErrorNoneAvailable
};

#endif
