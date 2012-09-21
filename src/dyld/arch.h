#ifndef ARCH_H
#define ARCH_H

#if defined(__x86_64__)
#	define ARCH_NAME		"x86-64"
#	define ARCH_CROSS_NAME	"i386"
#	define DYLD_FULL_NAME	"dyld64"
#	define DYLD_CROSS_NAME	"dyld32"

typedef uint64_t intptr;

#elif defined(__i386__)
#	define ARCH_NAME		"i386"
#	define ARCH_CROSS_NAME	"x86-64"
#	define DYLD_FULL_NAME	"dyld32"
#	define DYLD_CROSS_NAME	"dyld64"

typedef uint32_t intptr;

#else
#	error Unsupported platform
#endif

#endif
