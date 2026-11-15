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
#define HAVE_CPUID_H
#define HAVE_GCC_GET_CPUID
#define HAVE_PTHREAD_SETSCHEDPARAM
#define HAVE_PTHREAD_SETNAME_NP
#define PTHREAD_SETNAME_NP_ONE_PARAM

#ifdef __LP64__
#define SIZEOF_LONG 8
#else
#define SIZEOF_LONG 4
#endif
