#ifndef UNICHAR_H
#define UNICHAR_H
#include "MacTypes.h"
#include <endian.h>

// UniChar uses platform endianness, whereas ICU expects BE by default
#if __BYTE_ORDER == __BIG_ENDIAN
#	define UNICHAR_ENCODING "UTF-16BE"
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#	define UNICHAR_ENCODING "UTF-16LE"
#else
#	error Cannot determine endianness!
#endif

#endif

