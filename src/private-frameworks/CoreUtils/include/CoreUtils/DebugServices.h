/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 1997-2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//---------------------------------------------------------------------------------------------------------------------------
/*!	@header		DebugServices

    Debugging Library
 */

#ifndef __DEBUG_SERVICES__
#define __DEBUG_SERVICES__

#include    <stdarg.h>

#include    "CommonServices.h"

#if 0
#pragma mark == Settings ==
#endif

//===========================================================================================================================
//	Settings
//===========================================================================================================================

// General

#if ( !defined( DEBUG ) )
    #define DEBUG       0
#endif

#if ( defined( NDEBUG ) && DEBUG )
    #error NDEBUG defined and DEBUG is also enabled...they need to be in-sync
#endif

// AssertMacros.h/Debugging.h overrides.

#if ( !defined( DEBUG_OVERRIDE_APPLE_MACROS ) )
    #define DEBUG_OVERRIDE_APPLE_MACROS     1
#endif

// Routine name. Uses ISO __func__ where possible. Otherwise, uses the best thing that is available (if anything).

#if ( defined( __MWERKS__ ) || ( __GNUC__ > 2 ) || ( ( __GNUC__ == 2 ) && ( __GNUC_MINOR__ >= 9 ) ) )
    #define __ROUTINE__                 __func__
#elif ( defined( __GNUC__ ) )
    #define __ROUTINE__                 __PRETTY_FUNCTION__
#elif ( defined( _MSC_VER ) && !defined( _WIN32_WCE ) )
    #define __ROUTINE__                 __FUNCTION__
#else
    #define __ROUTINE__                 ""
#endif

// Variable argument macro support. Use ANSI C99 __VA_ARGS__ where possible. Otherwise, use the next best thing.

#if ( defined( __GNUC__ ) )
    #if ( ( __GNUC__ > 3 ) || ( ( __GNUC__ == 3 ) && ( __GNUC_MINOR__ >= 3) ) )
        #define DEBUG_C99_VA_ARGS       1
        #define DEBUG_GNU_VA_ARGS       0
    #else
        #define DEBUG_C99_VA_ARGS       0
        #define DEBUG_GNU_VA_ARGS       1
    #endif
#elif ( defined( __MWERKS__ ) )
    #define DEBUG_C99_VA_ARGS           1
    #define DEBUG_GNU_VA_ARGS           0
#else
    #define DEBUG_C99_VA_ARGS           0
    #define DEBUG_GNU_VA_ARGS           0
#endif

#if 0
#pragma mark == Output ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_FPRINTF_ENABLED

    @abstract	Enables ANSI C fprintf output.
 */

#if ( !defined( DEBUG_FPRINTF_ENABLED ) )
    #if ( !TARGET_API_MAC_OSX_KERNEL && !TARGET_OS_WINDOWS_CE )
        #define DEBUG_FPRINTF_ENABLED           1
    #else
        #define DEBUG_FPRINTF_ENABLED           0
    #endif
#else
    #if ( TARGET_API_MAC_OSX_KERNEL || TARGET_OS_WINDOWS_CE )
        #error fprintf enabled, but not supported on Mac OS X kernel or Windows CE
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_MAC_OS_X_IOLOG_ENABLED

    @abstract	Enables IOLog (Mac OS X Kernel) output.
 */

#if ( !defined( DEBUG_MAC_OS_X_IOLOG_ENABLED ) )
    #define DEBUG_MAC_OS_X_IOLOG_ENABLED        TARGET_API_MAC_OSX_KERNEL
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_KPRINTF_ENABLED

    @abstract	Enables kprintf (Mac OS X Kernel) output.
 */

#if ( !defined( DEBUG_KPRINTF_ENABLED ) )
    #define DEBUG_KPRINTF_ENABLED               TARGET_API_MAC_OSX_KERNEL
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_IDEBUG_ENABLED

    @abstract	Enables iDebug (Mac OS X user and Kernel) output.

    @discussion

    For Mac OS X kernel development, iDebug is enabled by default because we can dynamically check for the presence
    of iDebug via some exported IOKit symbols. Mac OS X app usage doesn't allow dynamic detection because it relies
    on statically linking to the iDebugServices.cp file so for Mac OS X app usage, you have to manually enable iDebug.
 */

#if ( !defined( DEBUG_IDEBUG_ENABLED ) )
    #define DEBUG_IDEBUG_ENABLED                TARGET_API_MAC_OSX_KERNEL
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_CORE_SERVICE_ASSERTS_ENABLED

    @abstract	Controls whether Core Services assert handling is enabled. Enabling requires CoreServices framework.
 */

#if ( !defined( DEBUG_CORE_SERVICE_ASSERTS_ENABLED ) )
    #if ( defined( __DEBUGGING__ ) )
        #define DEBUG_CORE_SERVICE_ASSERTS_ENABLED      1
    #else
        #define DEBUG_CORE_SERVICE_ASSERTS_ENABLED      0
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@typedef	DebugOutputType

    @abstract	Type of debug output (i.e. where the output goes).
 */

typedef uint32_t DebugOutputType;

#define kDebugOutputTypeNone                0x6E6F6E65U // 'none' - no params
#define kDebugOutputTypeCustom              0x63757374U // 'cust' - 1st param = function ptr, 2nd param = context
#define kDebugOutputTypeFPrintF             0x66707269U // 'fpri' - 1st param = DebugOutputTypeFlags [, 2nd param = filename]
#define kDebugOutputTypeiDebug              0x69646267U // 'idbg' - no params
#define kDebugOutputTypeKPrintF             0x6B707266U // 'kprf' - no params
#define kDebugOutputTypeMacOSXIOLog         0x696C6F67U // 'ilog' - no params
#define kDebugOutputTypeMacOSXLog           0x786C6F67U // 'xlog' - no params
#define kDebugOutputTypeWindowsDebugger     0x77696E64U // 'wind' - no params
#define kDebugOutputTypeWindowsEventLog     0x7765766CU // 'wevl' - 1st param = C-string name, 2nd param = HMODULE or NULL.

// Console meta output kind - Any kind of Console output (in horizontal order of preference):
//
// Mac OS X			= ANSI printf (viewable in Console.app)
// Mac OS X Kernel	= IOLog (/var/log/system.log) or kprintf (serial).
// Windows			= ANSI printf (Console window) or OutputDebugString (debugger).
// Other			= ANSI printf (viewer varies).

#define kDebugOutputTypeMetaConsole         0x434F4E53U // 'CONS' - no params

//---------------------------------------------------------------------------------------------------------------------------
/*!	@typedef	DebugOutputTypeFlags

    @abstract	Flags controlling how the output type is configured.

    @constant	kDebugOutputTypeFlagsTypeMask	Bit mask for the output type (e.g. stdout, stderr, file, etc.).
    @constant	kDebugOutputTypeFlagsStdOut		fprintf should go to stdout.
    @constant	kDebugOutputTypeFlagsStdErr		fprintf should go to stderr.
    @constant	kDebugOutputTypeFlagsFile		fprintf should go to a specific file (filename passed as va_arg).
 */

typedef unsigned int DebugOutputTypeFlags;

#define kDebugOutputTypeFlagsTypeMask   0xF
#define kDebugOutputTypeFlagsStdOut     1
#define kDebugOutputTypeFlagsStdErr     2
#define kDebugOutputTypeFlagsFile       10

//---------------------------------------------------------------------------------------------------------------------------
/*!	@typedef	DebugOutputFunctionPtr

    @abstract	Function ptr for a custom callback to print debug output.
 */

typedef void ( *DebugOutputFunctionPtr )( char *inData, size_t inSize, void *inContext );

//===========================================================================================================================
//	Constants
//===========================================================================================================================

#if 0
#pragma mark == Flags ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@typedef	DebugFlags

    @abstract	Flags controlling how output is printed.
 */

typedef uint32_t DebugFlags;

#define kDebugFlagsNone                 0
#define kDebugFlagsNoAddress            ( 1 << 0 )
#define kDebugFlagsNoOffset             ( 1 << 1 )
#define kDebugFlags32BitOffset          ( 1 << 2 )
#define kDebugFlagsNoASCII              ( 1 << 3 )
#define kDebugFlagsNoNewLine            ( 1 << 4 )
#define kDebugFlags8BitSeparator        ( 1 << 5 )
#define kDebugFlags16BitSeparator       ( 1 << 6 )
#define kDebugFlagsNo32BitSeparator     ( 1 << 7 )
#define kDebugFlagsNo16ByteHexPad       ( 1 << 8 )
#define kDebugFlagsNoByteCount          ( 1 << 9 )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@enum		DebugTaskLevelFlags

    @abstract	Flags indicating the task level.
 */

enum
{
    kDebugInterruptLevelShift               = 0,
    kDebugInterruptLevelMask                = 0x00000007,
    kDebugInVBLTaskMask                     = 0x00000010,
    kDebugInDeferredTaskMask                = 0x00000020,
    kDebugInSecondaryInterruptHandlerMask   = 0x00000040,
    kDebugPageFaultFatalMask                = 0x00000100,   // There should be a "kPageFaultFatalMask" in Debugging.h.
    kDebugMPTaskLevelMask                   = 0x00000200,   // There should be a "kMPTaskLevelMask" in Debugging.h.
    kDebugInterruptDepthShift               = 16,
    kDebugInterruptDepthMask                = 0x00FF0000
};

#define DebugExtractTaskLevelInterruptLevel( LEVEL )    \
    ( ( ( LEVEL ) &kDebugInterruptLevelMask ) >> kDebugInterruptLevelShift )

#define DebugExtractTaskLevelInterruptDepth( LEVEL )    \
    ( ( ( LEVEL ) &kDebugInterruptDepthMask ) >> kDebugInterruptDepthShift )

#if 0
#pragma mark == Levels ==
#endif

//===========================================================================================================================
//	Constants & Types - Levels
//===========================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------
/*!	@typedef	DebugLevel

    @abstract	Level used to control debug logging.
 */

typedef int32_t DebugLevel;

// Levels

#define kDebugLevelMask                 0x0000FFFF
#define kDebugLevelChatty               100
#define kDebugLevelVerbose              500
#define kDebugLevelTrace                800
#define kDebugLevelInfo                 1000
#define kDebugLevelNotice               3000
#define kDebugLevelWarning              5000
#define kDebugLevelAssert               6000
#define kDebugLevelRequire              7000
#define kDebugLevelError                8000
#define kDebugLevelCritical             9000
#define kDebugLevelAlert                10000
#define kDebugLevelEmergency            11000
#define kDebugLevelTragic               12000
#define kDebugLevelMax                  0x0000FFFF

// Level Flags

#define kDebugLevelFlagMask             0xFFFF0000
#define kDebugLevelFlagStackTrace       0x00010000
#define kDebugLevelFlagDebugBreak       0x00020000

//---------------------------------------------------------------------------------------------------------------------------
/*!	@typedef	LogLevel

    @abstract	Level used to control which events are logged.
 */

typedef int32_t LogLevel;

#define kLogLevelUninitialized      -1L
#define kLogLevelAll                0L
#define kLogLevelChatty             100L
#define kLogLevelVerbose            500L
#define kLogLevelTrace              800L
#define kLogLevelInfo               1000L
#define kLogLevelNotice             3000L
#define kLogLevelWarning            4000L
#define kLogLevelAssert             6000L
#define kLogLevelRequire            7000L
#define kLogLevelError              8000L
#define kLogLevelCritical           9000L
#define kLogLevelAlert              10000L
#define kLogLevelEmergency          11000L
#define kLogLevelTragic             12000L
#define kLogLevelOff                0x0000FFFEL

#if 0
#pragma mark == Properties ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@typedef	DebugPropertyTag

    @abstract	Tag for properties.
 */

typedef uint32_t DebugPropertyTag;

#define kDebugPropertyTagPrintLevelMin      0x6D696E70U     // 'minp'	Get: 1st param = DebugLevel *
                                                            //			Set: 1st param = DebugLevel

#define kDebugPropertyTagPrintLevel         kDebugPropertyTagPrintLevelMin

#define kDebugPropertyTagPrintLevelMax      0x706D786CU     // 'maxp'	Get: 1st param = DebugLevel *
                                                            //			Set: 1st param = DebugLevel

#define kDebugPropertyTagBreakLevel         0x62726B6CU     // 'brkl'	Get: 1st param = DebugLevel *
                                                            //			Set: 1st param = DebugLevel
#if 0
#pragma mark == General macros ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_UNUSED

    @abstract	Macro to mark a paramter as unused to avoid unused parameter warnings.

    @discussion

    There is no universally supported pragma/attribute for indicating a variable is unused. DEBUG_UNUSED lets us
    indicate a variable is unused in a manner that is supported by most compilers.
 */

#define DEBUG_UNUSED( X )           (void)( X )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_USE_ONLY

    @abstract	Macro to mark a variable as used only when debugging is enabled.

    @discussion

    Variables are sometimes needed only for debugging. When debugging is turned off, these debug-only variables generate
    compiler warnings about unused variables. To eliminate these warnings, use these macros to indicate variables that
    are only used for debugging.
 */

#if ( DEBUG )
    #define DEBUG_USE_ONLY( X )
#else
    #define DEBUG_USE_ONLY( X )     (void)( X )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_LOCAL

    @abstract	Macros to make variables and functions static when debugging is off, but extern when debugging is on.

    @discussion

    Rather than using "static" directly, using this macros allows you to access these variables external while
    debugging without being penalized for production builds.
 */

#if ( DEBUG )
    #define DEBUG_LOCAL
#else
    #define DEBUG_LOCAL         static
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_STATIC

    @abstract	Macros to make variables and functions static when debugging is off, but extern when debugging is on.

    @discussion

    Rather than using "static" directly, using this macros allows you to access these variables external while
    debugging without being penalized for production builds.
 */

#if ( DEBUG )
    #define DEBUG_STATIC
#else
    #define DEBUG_STATIC    static
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DEBUG_EXPORT

    @abstract	Macros to export variables.

    @discussion

    "__private_extern__" is a hack for IOKit to allow symbols to be exported from compilation units, but
    // not exported outside a driver (IOKit uses a lame global namespace for symbols). This still does not
    // solve the problem of multiple drivers in the same dependency chain since they share symbols.
 */

#if ( TARGET_API_MAC_OSX_KERNEL )
    #define DEBUG_EXPORT        __private_extern__
#else
    #define DEBUG_EXPORT        extern
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_add

    @abstract	Macro to add (or subtract if negative) a value when debugging is on. Does nothing if debugging is off.
 */

#if ( DEBUG )
    #define debug_add( A, B )       ( A ) += ( B )
#else
    #define debug_add( A, B )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_perform

    @abstract	Macro to perform something in debug-only builds.
 */

#if ( DEBUG )
    #define debug_perform( X )      do { X; } while( 0 )
#else
    #define debug_perform( X )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	translate_errno

    @abstract	Returns 0 if the test success. If the test fails, returns errno if non-zero and othewise the alternate error.
 */

#define translate_errno( TEST, ERRNO, ALTERNATE_ERROR )     ( ( TEST ) ? 0 : ( ERRNO ) ? ( ERRNO ) : ( ALTERNATE_ERROR ) )

#if 0
#pragma mark == Compile Time macros ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_compile_time

    @abstract	Performs a compile-time check of something such as the size of an int.

    @discussion

    This declares an array with a size that is determined by a compile-time expression. If the expression evaluates
    to 0, the array has a size of -1, which is illegal and generates a compile-time error.

    For example:

    check_compile_time( sizeof( int ) == 4 );

    Note: This only works with compile-time expressions.
    Note: This only works in places where extern declarations are allowed (e.g. global scope).

    References:

    <http://www.jaggersoft.com/pubs/CVu11_3.html>
    <http://www.jaggersoft.com/pubs/CVu11_5.html>

    Note: The following macros differ from the macros on the www.jaggersoft.com web site because those versions do not
    work with GCC due to GCC allow a zero-length array. Using a -1 condition turned out to be more portable.
 */

#define check_compile_time( X )     extern int debug_compile_time_name[ ( X ) ? 1 : -1 ]

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_compile_time_code

    @abstract	Perform a compile-time check, suitable for placement in code, of something such as the size of an int.

    @discussion

    This creates a switch statement with an existing case for 0 and an additional case using the result of a
    compile-time expression. A switch statement cannot have two case labels with the same constant so if the
    compile-time expression evaluates to 0, it is illegal and generates a compile-time error. If the compile-time
    expression does not evaluate to 0, the resulting value is used as the case label and it compiles without error.

    For example:

    check_compile_time_code( sizeof( int ) == 4 );

    Note: This only works with compile-time expressions.
    Note: This does not work in a global scope so it must be inside a function.

    References:

    <http://www.jaggersoft.com/pubs/CVu11_3.html>
    <http://www.jaggersoft.com/pubs/CVu11_5.html>
 */

#define check_compile_time_code( X )    switch( 0 ) { case 0: case X:; }

#if 0
#pragma mark == check macros ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check

    @abstract	Check that an expression is true (non-zero).

    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method.

    Code inside check() statements is not compiled into production builds.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef check
#endif
#if ( !defined( check ) )
    #if ( DEBUG )
        #define check( X )                                                                                  \
    do                                                                                              \
    {                                                                                               \
        if( !( X ) )                                                                                \
        {                                                                                           \
            debug_print_assert( 0, # X, NULL, __FILE__, __LINE__, __ROUTINE__ );                     \
        }                                                                                           \
    } while( 0 )
    #else
        #define check( X )
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_string

    @abstract	Check that an expression is true (non-zero) with an explanation.

    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) and a custom explanation string using the default debugging output method.

    Code inside check_string() statements is not compiled into production builds.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef check_string
#endif
#if ( !defined( check_string ) )
    #if ( DEBUG )
        #define check_string( X, STR )                                                                      \
    do                                                                                              \
    {                                                                                               \
        if( !( X ) )                                                                                \
        {                                                                                           \
            debug_print_assert( 0, # X, STR, __FILE__, __LINE__, __ROUTINE__ );                      \
        }                                                                                           \
                                                                                                            \
    }   while( 0 )
    #else
        #define check_string( X, STR )
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_noerr

    @abstract	Check that an error code is noErr (0).

    @discussion

    If the error code is non-0, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method.

    Code inside check_noerr() statements is not compiled into production builds.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef check_noerr
#endif
#if ( !defined( check_noerr ) )
    #if ( DEBUG )
        #define check_noerr( ERR )                                                                          \
    do                                                                                              \
    {                                                                                               \
        int_least32_t localErr;                                                               \
                                                                                                            \
        localErr = (int_least32_t)( ERR );                                                          \
        if( localErr != 0 )                                                                         \
        {                                                                                           \
            debug_print_assert( localErr, NULL, NULL, __FILE__, __LINE__, __ROUTINE__ );            \
        }                                                                                           \
                                                                                                            \
    }   while( 0 )
    #else
        #define check_noerr( ERR )
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_noerr_string

    @abstract	Check that an error code is noErr (0) with an explanation.

    @discussion

    If the error code is non-0, this prints debugging information (actual expression string, file, line number,
    function name, etc.) and a custom explanation string using the default debugging output method.

    Code inside check_noerr_string() statements is not compiled into production builds.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef check_noerr_string
#endif
#if ( !defined( check_noerr_string ) )
    #if ( DEBUG )
        #define check_noerr_string( ERR, STR )                                                              \
    do                                                                                              \
    {                                                                                               \
        int_least32_t localErr;                                                               \
                                                                                                            \
        localErr = (int_least32_t)( ERR );                                                          \
        if( localErr != 0 )                                                                         \
        {                                                                                           \
            debug_print_assert( localErr, NULL, STR, __FILE__, __LINE__, __ROUTINE__ );             \
        }                                                                                           \
                                                                                                            \
    }   while( 0 )
    #else
        #define check_noerr_string( ERR, STR )
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_translated_errno

    @abstract	Check a condition and prints errno (if non-zero) to the log.

    @discussion

    Code inside check_translated_errno() statements is not compiled into production builds.
 */

#if ( !defined( check_translated_errno ) )
    #if ( DEBUG )
        #define check_translated_errno( TEST, ERRNO, ALTERNATE_ERROR )                                      \
    do                                                                                              \
    {                                                                                               \
        if( !( TEST ) )                                                                             \
        {                                                                                           \
            int_least32_t localErr;                                                           \
                                                                                                            \
            localErr = (int_least32_t)( ERRNO );                                                    \
            localErr = ( localErr != 0 ) ? localErr : (int_least32_t)( ALTERNATE_ERROR );           \
            debug_print_assert( localErr, # TEST, NULL, __FILE__, __LINE__, __ROUTINE__ );           \
        }                                                                                           \
                                                                                                            \
    }   while( 0 )
    #else
        #define check_translated_errno( TEST, ERRNO, ALTERNATE_ERROR )
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	check_ptr_overlap

    @abstract	Checks that two ptrs do not overlap.
 */

#define check_ptr_overlap( P1, P1_SIZE, P2, P2_SIZE )                                       \
    do                                                                                      \
    {                                                                                       \
        check( !( ( (uintptr_t)( P1 ) >=     (uintptr_t)( P2 ) ) &&                         \
                  ( (uintptr_t)( P1 ) <  ( ( (uintptr_t)( P2 ) ) + ( P2_SIZE ) ) ) ) );     \
        check( !( ( (uintptr_t)( P2 ) >=     (uintptr_t)( P1 ) ) &&                         \
                  ( (uintptr_t)( P2 ) <  ( ( (uintptr_t)( P1 ) ) + ( P1_SIZE ) ) ) ) );     \
                                                                                            \
    }   while( 0 )

#if 0
#pragma mark == require macros ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require

    @abstract	Requires that an expression evaluate to true.

    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method then jumps to a label.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require
#endif
#if ( !defined( require ) )
    #define require( X, LABEL )                                                                             \
    do                                                                                                  \
    {                                                                                                   \
        if( !( X ) )                                                                                    \
        {                                                                                               \
            debug_print_assert( 0, # X, NULL, __FILE__, __LINE__, __ROUTINE__ );                         \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_string

    @abstract	Requires that an expression evaluate to true with an explanation.

    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) and a custom explanation string using the default debugging output method then jumps to a label.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_string
#endif
#if ( !defined( require_string ) )
    #define require_string( X, LABEL, STR )                                                                 \
    do                                                                                                  \
    {                                                                                                   \
        if( !( X ) )                                                                                    \
        {                                                                                               \
            debug_print_assert( 0, # X, STR, __FILE__, __LINE__, __ROUTINE__ );                          \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_quiet

    @abstract	Requires that an expression evaluate to true.

    @discussion

    If expression evalulates to false, this jumps to a label. No debugging information is printed.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_quiet
#endif
#if ( !defined( require_quiet ) )
    #define require_quiet( X, LABEL )                                                                       \
    do                                                                                                  \
    {                                                                                                   \
        if( !( X ) )                                                                                    \
        {                                                                                               \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr

    @abstract	Require that an error code is noErr (0).

    @discussion

    If the error code is non-0, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method then jumps to a label.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_noerr
#endif
#if ( !defined( require_noerr ) )
    #define require_noerr( ERR, LABEL )                                                                     \
    do                                                                                                  \
    {                                                                                                   \
        int_least32_t localErr;                                                                   \
                                                                                                            \
        localErr = (int_least32_t)( ERR );                                                              \
        if( localErr != 0 )                                                                             \
        {                                                                                               \
            debug_print_assert( localErr, NULL, NULL, __FILE__, __LINE__, __ROUTINE__ );                \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr_string

    @abstract	Require that an error code is noErr (0).

    @discussion

    If the error code is non-0, this prints debugging information (actual expression string, file, line number,
    function name, etc.), and a custom explanation string using the default debugging output method using the
    default debugging output method then jumps to a label.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_noerr_string
#endif
#if ( !defined( require_noerr_string ) )
    #define require_noerr_string( ERR, LABEL, STR )                                                         \
    do                                                                                                  \
    {                                                                                                   \
        int_least32_t localErr;                                                                   \
                                                                                                            \
        localErr = (int_least32_t)( ERR );                                                              \
        if( localErr != 0 )                                                                             \
        {                                                                                               \
            debug_print_assert( localErr, NULL, STR, __FILE__, __LINE__, __ROUTINE__ );                 \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr_action_string

    @abstract	Require that an error code is noErr (0).

    @discussion

    If the error code is non-0, this prints debugging information (actual expression string, file, line number,
    function name, etc.), and a custom explanation string using the default debugging output method using the
    default debugging output method then executes an action and jumps to a label.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_noerr_action_string
#endif
#if ( !defined( require_noerr_action_string ) )
    #define require_noerr_action_string( ERR, LABEL, ACTION, STR )                                          \
    do                                                                                                  \
    {                                                                                                   \
        int_least32_t localErr;                                                                   \
                                                                                                            \
        localErr = (int_least32_t)( ERR );                                                              \
        if( localErr != 0 )                                                                             \
        {                                                                                               \
            debug_print_assert( localErr, NULL, STR, __FILE__, __LINE__, __ROUTINE__ );                 \
            { ACTION; }                                                                                 \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr_quiet

    @abstract	Require that an error code is noErr (0).

    @discussion

    If the error code is non-0, this jumps to a label. No debugging information is printed.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_noerr_quiet
#endif
#if ( !defined( require_noerr_quiet ) )
    #define require_noerr_quiet( ERR, LABEL )                                                               \
    do                                                                                                  \
    {                                                                                                   \
        if( ( ERR ) != 0 )                                                                              \
        {                                                                                               \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr_action

    @abstract	Require that an error code is noErr (0) with an action to execute otherwise.

    @discussion

    If the error code is non-0, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method then executes an action and jumps to a label.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_noerr_action
#endif
#if ( !defined( require_noerr_action ) )
    #define require_noerr_action( ERR, LABEL, ACTION )                                                      \
    do                                                                                                  \
    {                                                                                                   \
        int_least32_t localErr;                                                                   \
                                                                                                            \
        localErr = (int_least32_t)( ERR );                                                              \
        if( localErr != 0 )                                                                             \
        {                                                                                               \
            debug_print_assert( localErr, NULL, NULL, __FILE__, __LINE__, __ROUTINE__ );                \
            { ACTION; }                                                                                 \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_noerr_action_quiet

    @abstract	Require that an error code is noErr (0) with an action to execute otherwise.

    @discussion

    If the error code is non-0, this executes an action and jumps to a label. No debugging information is printed.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_noerr_action_quiet
#endif
#if ( !defined( require_noerr_action_quiet ) )
    #define require_noerr_action_quiet( ERR, LABEL, ACTION )                                                \
    do                                                                                                  \
    {                                                                                                   \
        if( ( ERR ) != 0 )                                                                              \
        {                                                                                               \
            { ACTION; }                                                                                 \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_action

    @abstract	Requires that an expression evaluate to true with an action to execute otherwise.

    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) using the default debugging output method then executes an action and jumps to a label.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_action
#endif
#if ( !defined( require_action ) )
    #define require_action( X, LABEL, ACTION )                                                              \
    do                                                                                                  \
    {                                                                                                   \
        if( !( X ) )                                                                                    \
        {                                                                                               \
            debug_print_assert( 0, # X, NULL, __FILE__, __LINE__, __ROUTINE__ );                         \
            { ACTION; }                                                                                 \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_action_quiet

    @abstract	Requires that an expression evaluate to true with an action to execute otherwise.

    @discussion

    If expression evalulates to false, this executes an action and jumps to a label. No debugging information is printed.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_action_quiet
#endif
#if ( !defined( require_action_quiet ) )
    #define require_action_quiet( X, LABEL, ACTION )                                                        \
    do                                                                                                  \
    {                                                                                                   \
        if( !( X ) )                                                                                    \
        {                                                                                               \
            { ACTION; }                                                                                 \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_action_string

    @abstract	Requires that an expression evaluate to true with an explanation and action to execute otherwise.

    @discussion

    If expression evalulates to false, this prints debugging information (actual expression string, file, line number,
    function name, etc.) and a custom explanation string using the default debugging output method then executes an
    action and jumps to a label.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef require_action_string
#endif
#if ( !defined( require_action_string ) )
    #define require_action_string( X, LABEL, ACTION, STR )                                                  \
    do                                                                                                  \
    {                                                                                                   \
        if( !( X ) )                                                                                    \
        {                                                                                               \
            debug_print_assert( 0, # X, STR, __FILE__, __LINE__, __ROUTINE__ );                      \
            { ACTION; }                                                                                 \
            goto LABEL;                                                                                 \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )

#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	require_throw

    @abstract	Requires that an expression evaluates to true or an exception is thrown.

    @discussion

    If the expression evaluates to false, this prints debugging information (actual expression string, file,
    line number, function name, etc.) using the default debugging output method then throws an exception.
 */

#if ( defined( __cplusplus ) )
    #define require_throw( X )                                                                              \
    do                                                                                                  \
    {                                                                                                   \
        if( !( X ) )                                                                                    \
        {                                                                                               \
            debug_print_assert( 0, # X, NULL, __FILE__, __LINE__, __ROUTINE__ );                         \
            throw kUnknownErr;                                                                          \
        }                                                                                               \
                                                                                                            \
    }   while( 0 )
#endif

#if 0
#pragma mark == Design-By-Contract macros ==
#endif

//===========================================================================================================================
//	Design-By-Contract macros
//===========================================================================================================================

#define ensure( X )                                                 check( X )
#define ensure_string( X, STR )                                     check_string( X, STR )
#define ensure_noerr( ERR )                                         check_noerr( ERR )
#define ensure_noerr_string( ERR, STR )                             check_noerr_string( ERR, STR )
#define ensure_translated_errno( TEST, ERRNO, ALTERNATE_ERROR )     check_translated_errno( TEST, ERRNO, ALTERNATE_ERROR )

// Note: Design-By-Contract "require" macros are already defined elsewhere.

#if 0
#pragma mark == Expect macros ==
#endif

//===========================================================================================================================
//	Expect macros
//===========================================================================================================================

// Expect macros allow code to include runtime checking of things that should not happen in shipping code (e.g. internal
// programmer errors, such as a NULL parameter where it is not allowed). Once the code has been verified to work correctly
// without asserting, the DEBUG_EXPECT_VERIFIED conditional can be set to eliminate the error checking entirely. It can
// also be useful to measure the cost of error checking code by profiling with it enable and with it disabled.

#if ( DEBUG_EXPECT_VERIFIED )
    #define require_expect
    #define require_string_expect
    #define require_quiet_expect
    #define require_noerr_expect
    #define require_noerr_string_expect
    #define require_noerr_action_string_expect
    #define require_noerr_quiet_expect
    #define require_noerr_action_expect
    #define require_noerr_action_quiet_expect
    #define require_action_expect
    #define require_action_quiet_expect
    #define require_action_string_expect
#else
    #define require_expect                          require
    #define require_string_expect                   require_string
    #define require_quiet_expect                    require_quiet
    #define require_noerr_expect                    require_noerr
    #define require_noerr_string_expect             require_noerr_string
    #define require_noerr_action_string_expect      require_noerr_action_string
    #define require_noerr_quiet_expect              require_noerr_quiet
    #define require_noerr_action_expect             require_noerr_action
    #define require_noerr_action_quiet_expect       require_noerr_action_quiet
    #define require_action_expect                   require_action
    #define require_action_quiet_expect             require_action_quiet
    #define require_action_string_expect            require_action_string
#endif

#if 0
#pragma mark == Output macros ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_string

    @abstract	Prints a debugging C string.
 */

#if ( DEBUG_OVERRIDE_APPLE_MACROS )
    #undef debug_string
#endif
#if ( !defined( debug_string ) )
    #if ( DEBUG )
        #define debug_string( STR )                                                                     \
    do                                                                                          \
    {                                                                                           \
        debug_print_assert( 0, NULL, STR, __FILE__, __LINE__, __ROUTINE__ );                    \
                                                                                                        \
    }   while( 0 )
    #else
        #define debug_string( STR )
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	debug_print_assert

    @abstract	Prints an assertion.
 */

#if ( DEBUG )
    #define debug_print_assert( ERROR_CODE, ASSERT_STRING, MESSAGE, FILENAME, LINE_NUMBER, FUNCTION )   \
    DebugPrintAssert( ERROR_CODE, ASSERT_STRING, MESSAGE, FILENAME, LINE_NUMBER, FUNCTION )
#else
    #define debug_print_assert( ERROR_CODE, ASSERT_STRING, MESSAGE, FILENAME, LINE_NUMBER, FUNCTION )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	dlog

    @abstract	Prints a debug-only message.
 */

#if ( DEBUG )
    #if ( DEBUG_C99_VA_ARGS )
        #define dlog(... )         DebugPrintF( __VA_ARGS__ )
    #elif ( DEBUG_GNU_VA_ARGS )
        #define dlog( ARGS... )     DebugPrintF( ## ARGS )
    #else
        #define dlog                DebugPrintF
    #endif
#else
    #if ( DEBUG_C99_VA_ARGS )
        #define dlog(... )
    #elif ( DEBUG_GNU_VA_ARGS )
        #define dlog( ARGS... )
    #else
        #define dlog                while( 0 )
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	dlogv

    @abstract	Prints a debug-only message.
 */

#if ( DEBUG )
    #define dlogv( LEVEL, FORMAT, LIST )        DebugPrintFVAList( ( LEVEL ), ( FORMAT ), ( LIST ) )
#else
    #define dlogv( LEVEL, FORMAT, LIST )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	dlogmem

    @abstract	Prints a debug-only dump of memory.
 */

#if ( DEBUG )
    #define dlogmem( LEVEL, PTR, SIZE )     \
    DebugHexDump( ( LEVEL ), 0, NULL, 0, 0, NULL, 0, ( PTR ), ( PTR ), ( SIZE ), kDebugFlagsNone, NULL, 0 )
#else
    #define dlogmem( LEVEL, PTR, SIZE )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	DebugNSLog

    @abstract	Debug-only macro for the Cocoa NSLog function.
 */

#if ( DEBUG )
    #if ( DEBUG_C99_VA_ARGS )
        #define DebugNSLog(... )           NSLog( __VA_ARGS__ )
    #elif ( DEBUG_GNU_VA_ARGS )
        #define DebugNSLog( ARGS... )       NSLog( ## ARGS )
    #else
        #define DebugNSLog                  NSLog
    #endif
#else
    #if ( DEBUG_C99_VA_ARGS )
        #define DebugNSLog(... )
    #elif ( DEBUG_GNU_VA_ARGS )
        #define DebugNSLog( ARGS... )
    #else
        #define DebugNSLog                  while( 0 )
    #endif
#endif

#if 0
#pragma mark == Routines - General ==
#endif

#ifdef  __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugInitialize

    @abstract	Initializes the debugging library for a specific kind of output.

    @param		inType
    @param		varArg		Variable number parameters, controlled by the "inType" parameter.
 */

#if ( DEBUG )
DEBUG_EXPORT OSStatus   DebugInitialize( DebugOutputType inType, ... );
#endif

#if ( DEBUG )
    #if ( DEBUG_C99_VA_ARGS )
        #define debug_initialize(... )         DebugInitialize( __VA_ARGS__ )
    #elif ( DEBUG_GNU_VA_ARGS )
        #define debug_initialize( ARGS... )     DebugInitialize( ## ARGS )
    #else
        #define debug_initialize                DebugInitialize
    #endif
#else
    #if ( DEBUG_C99_VA_ARGS )
        #define debug_initialize(... )
    #elif ( DEBUG_GNU_VA_ARGS )
        #define debug_initialize( ARGS... )
    #else
        #define debug_initialize                while( 0 )
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugFinalize

    @abstract	Releases any resources used by the debugging library
 */

#if ( DEBUG )
DEBUG_EXPORT void       DebugFinalize( void );
#endif

#if ( DEBUG )
    #define debug_terminate()   DebugFinalize()
#else
    #define debug_terminate()
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugGetProperty

    @abstract	Gets the specified property from the debugging library.
 */

#if ( DEBUG )
DEBUG_EXPORT OSStatus   DebugGetProperty( DebugPropertyTag inTag, ... );
#endif

#if ( DEBUG )
    #if ( DEBUG_C99_VA_ARGS )
        #define debug_get_property(... )           DebugGetProperty( __VA_ARGS__ )
    #elif ( DEBUG_GNU_VA_ARGS )
        #define debug_get_property( ARGS... )       DebugGetProperty( ## ARGS )
    #else
        #define debug_get_property                  DebugGetProperty
    #endif
#else
    #if ( DEBUG_C99_VA_ARGS )
        #define debug_get_property(... )
    #elif ( DEBUG_GNU_VA_ARGS )
        #define debug_get_property( ARGS... )
    #else
        #define debug_get_property                  while( 0 )
    #endif
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugSetProperty

    @abstract	Sets the specified property from the debugging library.
 */

#if ( DEBUG )
DEBUG_EXPORT OSStatus   DebugSetProperty( DebugPropertyTag inTag, ... );
#endif

#if ( DEBUG )
    #if ( DEBUG_C99_VA_ARGS )
        #define debug_set_property(... )           DebugSetProperty( __VA_ARGS__ )
    #elif ( DEBUG_GNU_VA_ARGS )
        #define debug_set_property( ARGS... )       DebugSetProperty( ## ARGS )
    #else
        #define debug_set_property                  DebugSetProperty
    #endif
#else
    #if ( DEBUG_C99_VA_ARGS )
        #define debug_set_property(... )
    #elif ( DEBUG_GNU_VA_ARGS )
        #define debug_set_property( ARGS... )
    #else
        #define debug_set_property                  while( 0 )
    #endif
#endif

#if 0
#pragma mark == Routines - Debugging Output ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugPrintF

    @abstract	Prints a debug message with printf-style formatting.

    @param		inLevel	Error that generated this assert or noErr.

    @param		inFormatString
                    C string containing assertion text.

    @param		VAR_ARG
                    Variable number of arguments depending on the format string.

    @result		Number of bytes printed or -1 on error.
 */

#if ( DEBUG )
DEBUG_EXPORT size_t     DebugPrintF( DebugLevel inLevel, const char *inFormat, ... );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugPrintFVAList

    @abstract	va_list version of DebugPrintF. See DebugPrintF for more info.
 */

#if ( DEBUG )
DEBUG_EXPORT size_t     DebugPrintFVAList( DebugLevel inLevel, const char *inFormat, va_list inArgs );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugPrintAssert

    @abstract	Prints a message describing the reason the (e.g. an assert failed), an optional error message,
                an optional source filename, an optional source line number.

    @param		inErrorCode			Error that generated this assert or noErr.
    @param		inAssertString		C string containing assertion text.
    @param		inMessage			C string containing a message about the assert.
    @param		inFileName			C string containing path of file where the error occurred.
    @param		inLineNumber		Line number in source file where the error occurred.
    @param		inFunction			C string containing name of function where assert occurred.

    @discussion

    Example output:

    [ASSERT] assert: "dataPtr != NULL" allocate memory for object failed
    [ASSERT] where:  "MyFile.c", line 123, ("MyFunction")

    OR

    [ASSERT] error:  -6728 (kNoMemoryErr)
    [ASSERT] where:  "MyFile.c", line 123, ("MyFunction")
 */

#if ( DEBUG )
DEBUG_EXPORT void
DebugPrintAssert(
    int_least32_t inErrorCode,
    const char *    inAssertString,
    const char *    inMessage,
    const char *    inFilename,
    int_least32_t inLineNumber,
    const char *    inFunction );
#endif

#if 0
#pragma mark == Routines - Utilities ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugSNPrintF

    @abstract	Debugging versions of standard C snprintf with extra features.

    @param		sbuffer		Buffer to receive result. Null terminated unless the buffer size is 0.
    @param		buflen		Size of the buffer including space for the null terminator.
    @param		fmt			printf-style format string.
    @param		VAR_ARG		Variable number of arguments depending on the format string.

    @result		Number of characters written (minus the null terminator).

    @discussion

    Extra features over the standard C snprintf:
    <pre>
        64-bit support for %d (%lld), %i (%lli), %u (%llu), %o (%llo), %x (%llx), and %b (%llb).
        %@   - Cocoa/CoreFoundation object. Param is the object. Strings are used directly. Others use CFCopyDescription.
        %a   - Network Address: %.4a=IPv4, %.6a=Ethernet, %.8a Fibre Channel, %.16a=IPv6. Arg=ptr to network address.
        %#a  - IPv4 or IPv6 mDNSAddr. Arg=ptr to mDNSAddr.
        %##a - IPv4 (if AF_INET defined) or IPv6 (if AF_INET6 defined) sockaddr. Arg=ptr to sockaddr.
        %b   - Binary representation of integer (e.g. 01101011). Modifiers and arg=the same as %d, %x, etc.
        %C   - Mac-style FourCharCode (e.g. 'APPL'). Arg=32-bit value to print as a Mac-style FourCharCode.
        %H   - Hex Dump (e.g. "\x6b\xa7" -> "6B A7"). 1st arg=ptr, 2nd arg=size, 3rd arg=max size.
        %#H  - Hex Dump & ASCII (e.g. "\x41\x62" -> "6B A7 'Ab'"). 1st arg=ptr, 2nd arg=size, 3rd arg=max size.
        %m   - Error Message (e.g. 0 -> "kNoErr"). Modifiers and error code arg=the same as %d, %x, etc.
        %#s  - Pascal-style length-prefixed string. Arg=ptr to string.
        %##s - DNS label-sequence name. Arg=ptr to name.
        %S   - UTF-16 string, 0x0000 terminated. Host order if no BOM. Precision is UTF-16 count. Precision includes BOM.
        %#S  - Big Endian UTF-16 string (unless BOM overrides). Otherwise, the same as %S.
        %##S - Little Endian UTF-16 string (unless BOM overrides). Otherwise, the same as %S.
        %U   - Universally Unique Identifier (UUID) (e.g. 6ba7b810-9dad-11d1-80b4-00c04fd430c8). Arg=ptr to 16-byte UUID.
    </pre>
 */

#if ( DEBUG )
DEBUG_EXPORT size_t DebugSNPrintF(char *sbuffer, size_t buflen, const char *fmt, ...);
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugSNPrintFVAList

    @abstract	va_list version of DebugSNPrintF. See DebugSNPrintF for more info.
 */

#if ( DEBUG )
DEBUG_EXPORT size_t DebugSNPrintFVAList(char *sbuffer, size_t buflen, const char *fmt, va_list arg);
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugGetErrorString

    @abstract	Gets an error string from an error code.

    @param		inStatus		Error code to get the string for.
    @param		inBuffer		Optional buffer to copy the string to for non-static strings. May be null.
    @param		inBufferSize	Size of optional buffer. May be 0.

    @result		C string containing error string for the error code. Guaranteed to be a valid, static string. If a
                buffer is supplied, the return value will always be a pointer to the supplied buffer, which will
                contain the best available description of the error code. If a buffer is not supplied, the return
                value will be the best available description of the error code that can be represented as a static
                string. This allows code that cannot use a temporary buffer to hold the result to still get a useful
                error string in most cases, but also allows code that can use a temporary buffer to get the best
                available description.
 */

#if ( DEBUG )
DEBUG_EXPORT const char *   DebugGetErrorString( int_least32_t inErrorCode, char *inBuffer, size_t inBufferSize );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugHexDump

    @abstract	Hex dumps data to a string or to the output device.
 */

#if ( DEBUG )
DEBUG_EXPORT size_t
DebugHexDump(
    DebugLevel inLevel,
    int inIndent,
    const char *    inLabel,
    size_t inLabelSize,
    int inLabelMinWidth,
    const char *    inType,
    size_t inTypeSize,
    const void *    inDataStart,
    const void *    inData,
    size_t inDataSize,
    DebugFlags inFlags,
    char *          outBuffer,
    size_t inBufferSize );
#endif

#if ( DEBUG )
    #define dloghex( LEVEL, INDENT, LABEL, LABEL_SIZE, LABEL_MIN_SIZE, TYPE, TYPE_SIZE, DATA_START, DATA, DATA_SIZE, FLAGS, BUFFER, BUFFER_SIZE )   \
    DebugHexDump( ( LEVEL ), (INDENT), ( LABEL ), ( LABEL_SIZE ), ( LABEL_MIN_SIZE ), ( TYPE ), ( TYPE_SIZE ),                              \
                  ( DATA_START ), ( DATA ), ( DATA_SIZE ), ( FLAGS ), ( BUFFER ), ( BUFFER_SIZE ) )
#else
    #define dloghex( LEVEL, INDENT, LABEL, LABEL_SIZE, LABEL_MIN_SIZE, TYPE, TYPE_SIZE, DATA_START, DATA, DATA_SIZE, FLAGS, BUFFER, BUFFER_SIZE )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugTaskLevel

    @abstract	Returns the current task level.

    @result		Current task level

    @discussion

    Bit masks to isolate portions of the result (note that some masks may also need bit shifts to right justify):
    <pre>
        kDebugInterruptLevelMask				- Indicates the current interrupt level (> 0 means interrupt time).
        kDebugInVBLTaskMask						- Indicates if a VBL task is currently being executed.
        kDebugInDeferredTaskMask				- Indicates if a Deferred Task is currently being executed.
        kDebugInSecondaryInterruptHandlerMask	- Indicates if a Secondary Interrupt Handler is currently being executed.
        kDebugPageFaultFatalMask				- Indicates if it is unsafe to cause a page fault (worse than interrupt time).
        kDebugMPTaskLevelMask					- Indicates if being called from an MP task.
        kDebugInterruptDepthMask				- 0 means task level, 1 means in interrupt, > 1 means in nested interrupt.
    </pre>

    Helpers:
    <pre>
        DebugExtractTaskLevelInterruptDepth()   - Macro to extract interrupt depth from task level value.
    </pre>
 */

#if ( DEBUG )
DEBUG_EXPORT uint32_t   DebugTaskLevel( void );
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	DebugServicesTest

    @abstract	Unit test.
 */

#if ( DEBUG )
DEBUG_EXPORT OSStatus   DebugServicesTest( void );
#endif

#ifdef  __cplusplus
}
#endif

#endif  // __DEBUG_SERVICES__
