#ifndef ARCH_H
#define ARCH_H

#if defined(__x86_64__)
#	define ARCH_NAME	"x86-64"

typedef uint64_t intptr;

#elif defined(__i386__)
#	define ARCH_NAME	"i386"

typedef uint32_t intptr;

#else
#	error Unsupported platform
#endif

#endif
