/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2008-2011 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
 


#ifndef INTERNAL_MACROS_H
#define INTERNAL_MACROS_H

#include <assert.h>
#include <Availability.h>

#ifdef __cplusplus
extern "C" {
#endif
	extern void __assert_rtn(const char *, const char *, int, const char *) __attribute__((noreturn));
#ifdef __cplusplus
}
#endif

#define UNW_STEP_SUCCESS 1
#define UNW_STEP_END     0


struct v128 { unsigned int vec[4]; };


#define EXPORT __attribute__((visibility("default"))) 

#define COMPILE_TIME_ASSERT( expr )    \
		extern int compile_time_assert_failed[ ( expr ) ? 1 : -1 ] __attribute__( ( unused ) );

#define ABORT(msg) __assert_rtn(__func__, __FILE__, __LINE__, msg) 

#if NDEBUG
	#define DEBUG_MESSAGE(msg, ...)  
	#define DEBUG_PRINT_API(msg, ...)
	#define DEBUG_PRINT_UNWINDING_TEST 0
	#define DEBUG_PRINT_UNWINDING(msg, ...)
	#define DEBUG_LOG_NON_ZERO(x) x;
	#define INITIALIZE_DEBUG_PRINT_API
	#define INITIALIZE_DEBUG_PRINT_UNWINDING
#else
	#define DEBUG_MESSAGE(msg, ...)  fprintf(stderr, "libuwind: " msg, __VA_ARGS__)
	#ifdef __cplusplus
		extern "C" {
	#endif
		extern  bool logAPIs();
		extern  bool logUnwinding();
	#ifdef __cplusplus
		}
	#endif
	#define DEBUG_LOG_NON_ZERO(x) { int _err = x; if ( _err != 0 ) fprintf(stderr, "libuwind: " #x "=%d in %s", _err, __FUNCTION__); }
	#define DEBUG_PRINT_API(msg, ...) do { if ( logAPIs() ) fprintf(stderr,  msg, __VA_ARGS__); } while(0)
	#define DEBUG_PRINT_UNWINDING(msg, ...) do { if ( logUnwinding() ) fprintf(stderr,  msg, __VA_ARGS__); } while(0)
	#define DEBUG_PRINT_UNWINDING_TEST logUnwinding()
	#define INITIALIZE_DEBUG_PRINT_API bool logAPIs() { static bool log = (getenv("LIBUNWIND_PRINT_APIS") != NULL); return log; }
	#define INITIALIZE_DEBUG_PRINT_UNWINDING bool logUnwinding() { static bool log = (getenv("LIBUNWIND_PRINT_UNWINDING") != NULL); return log; }
#endif


// static linker symbols to prevent wrong two level namespace for _Unwind symbols
#if __arm__
   #define NOT_HERE_BEFORE_5_0(sym)	   \
       extern const char sym##_tmp30 __asm("$ld$hide$os3.0$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp30 = 0; \
       extern const char sym##_tmp31 __asm("$ld$hide$os3.1$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp31 = 0; \
       extern const char sym##_tmp32 __asm("$ld$hide$os3.2$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp32 = 0; \
       extern const char sym##_tmp40 __asm("$ld$hide$os4.0$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp40 = 0; \
       extern const char sym##_tmp41 __asm("$ld$hide$os4.1$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp41 = 0; \
	   extern const char sym##_tmp42 __asm("$ld$hide$os4.2$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp42 = 0; \
	   extern const char sym##_tmp43 __asm("$ld$hide$os4.3$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp43 = 0;
#endif

#define NOT_HERE_BEFORE_10_6(sym) \
	extern const char sym##_tmp4 __asm("$ld$hide$os10.4$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp4 = 0; \
	extern const char sym##_tmp5 __asm("$ld$hide$os10.5$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp5 = 0; 
#define NEVER_HERE(sym) \
	extern const char sym##_tmp4 __asm("$ld$hide$os10.4$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp4 = 0; \
	extern const char sym##_tmp5 __asm("$ld$hide$os10.5$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp5 = 0; \
	extern const char sym##_tmp6 __asm("$ld$hide$os10.6$_" #sym ); __attribute__((visibility("default"))) const char sym##_tmp6 = 0;



#endif // INTERNAL_MACROS_H
