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

#elif defined(__arm__)
#	define ARCH_NAME		"arm"
#	define ARCH_CROSS_NAME	""
#	define DYLD_FULL_NAME	"dyld"
#	define DYLD_CROSS_NAME	"dyld"

typedef uint32_t intptr;

#elif defined(__powerpc__)
#	define ARCH_NAME		"ppc"
#   define ARCH_CROSS_NAME  "ppc64"
#   define DYLD_FULL_NAME   "dyld32"
#   define DYLD_CROSS_NAME  "dyld64"

typedef uint32_t intptr;

#elif defined(__powerpc64__)
#   define ARCH_NAME        "ppc64"
#   define ARCH_CROSS_NAME  "ppc"
#   define DYLD_FULL_NAME   "dyld64"
#   define DYLD_CROSS_NAME  "dyld32"


#else
#	error Unsupported platform
#endif

#endif
