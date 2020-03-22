/* Specify which architecture libffi is configured for. */
#ifdef __arm__
#ifndef ARM
#define ARM
#endif
#include <fficonfig_armv7.h>
#endif

#ifdef __arm64__
#ifndef AARCH64
#define AARCH64
#endif
#include <fficonfig_arm64.h>
#endif

#ifdef __i386__
#ifndef X86_ANY
#define X86_ANY
#endif
#ifndef X86_DARWIN
#define X86_DARWIN
#endif
#include <fficonfig_i386.h>
#endif

#ifdef __x86_64__
#ifndef X86_ANY
#define X86_ANY
#endif
#ifndef X86_64
#define X86_64
#endif
#include <fficonfig_x86_64.h>
#endif
