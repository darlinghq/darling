#ifndef ARCH_H
#define ARCH_H

#if defined(__x86_64__)
#	define ARCH_NAME	"x86-64"
#elif defined(__i386__)
#	define ARCH_NAME	"i386"
#else
#	error Unsupported platform
#endif

#endif
