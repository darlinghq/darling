#ifndef UNICHAR_H
#define UNICHAR_H
#include "MacTypes.h"
#include <TargetConditionals.h>

// UniChar uses platform endianness, whereas ICU expects BE by default
#if TARGET_RT_BIG_ENDIAN
#	define UNICHAR_ENCODING "UTF-16BE"
#elif TARGET_RT_LITTLE_ENDIAN
#	define UNICHAR_ENCODING "UTF-16LE"
#else
#	error Cannot determine endianness!
#endif

#endif

