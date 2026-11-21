/*
 * Configuration of the openal-soft sources as built for Darling's OpenAL
 * framework (replaces the header CMake would generate from config.h.in).
 * Copyright (C) 2026 Lubos Dolezel
 */

#define AL_API __attribute__((visibility("default")))
#define ALC_API __attribute__((visibility("default")))
#define RESTRICT __restrict

#define HAVE_POSIX_MEMALIGN
#define HAVE_PROC_PIDPATH
#define HAVE_COREAUDIO
#define HAVE_GCC_FORMAT
#define HAVE_DLFCN_H
#define HAVE_DIRENT_H
/* cpuid.h only describes x86 CPUID; clang ships it for every target but #error()s
 * on non-x86, so these must not be defined for arm64. */
#if defined(__i386__) || defined(__x86_64__)
#define HAVE_CPUID_H
#define HAVE_GCC_GET_CPUID
#endif
#define HAVE_PTHREAD_SETSCHEDPARAM
#define HAVE_PTHREAD_SETNAME_NP
#define PTHREAD_SETNAME_NP_ONE_PARAM

#ifdef __LP64__
#define SIZEOF_LONG 8
#else
#define SIZEOF_LONG 4
#endif
