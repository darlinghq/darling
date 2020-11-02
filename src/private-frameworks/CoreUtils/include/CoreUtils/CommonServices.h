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
// modified for Darling
// NOTE(@facekapow): i deleted the code for cross-platform compatability since Darling is for Darwin platforms only

#ifndef __COMMON_SERVICES__
#define __COMMON_SERVICES__

#include <os/base_private.h>
#include <TargetConditionals.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

#ifdef  __cplusplus
extern "C" {
#endif

// not sure if these belong here
#define CU_ASSUME_NONNULL_BEGIN OS_ASSUME_NONNULL_BEGIN
#define CU_ASSUME_NONNULL_END OS_ASSUME_NONNULL_END
#define likely(x) os_likely(x)
#define unlikely(x) os_unlikely(x)

#if 0
#pragma mark == Macros ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	kSizeCString

    @abstract	A meta-value to pass to supported routines to indicate the size should be calculated with strlen.
 */

#define kSizeCString        ( (size_t) -1 )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	sizeof_array

    @abstract	Determines the number of elements in an array.
 */

#define sizeof_array( X )       ( sizeof( X ) / sizeof( X[ 0 ] ) )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	sizeof_element

    @abstract	Determines the size of an array element.
 */

#define sizeof_element( X )     sizeof( X[ 0 ] )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	sizeof_string

    @abstract	Determines the size of a constant C string, excluding the null terminator.
 */

#define sizeof_string( X )      ( sizeof( ( X ) ) - 1 )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	sizeof_field

    @abstract	Determines the size of a field of a type.
 */

#define sizeof_field( TYPE, FIELD )     sizeof( ( ( (TYPE *) 0 )->FIELD ) )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	RoundUp

    @abstract	Rounds X up to a multiple of Y.
 */

#define RoundUp( X, Y )     ( ( X ) + ( ( Y ) -( ( X ) % ( Y ) ) ) )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	IsAligned

    @abstract	Returns non-zero if X is aligned to a Y byte boundary and 0 if not. Y must be a power of 2.
 */

#define IsAligned( X, Y )       ( ( ( X ) &( ( Y ) -1 ) ) == 0 )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	IsFieldAligned

    @abstract	Returns non-zero if FIELD of type TYPE is aligned to a Y byte boundary and 0 if not. Y must be a power of 2.
 */

#define IsFieldAligned( X, TYPE, FIELD, Y )     IsAligned( ( (uintptr_t)( X ) ) + offsetof( TYPE, FIELD ), ( Y ) )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	AlignDown

    @abstract	Aligns X down to a Y byte boundary. Y must be a power of 2.
 */

#define AlignDown( X, Y )       ( ( X ) &~( ( Y ) -1 ) )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	AlignUp

    @abstract	Aligns X up to a Y byte boundary. Y must be a power of 2.
 */

#define AlignUp( X, Y )     ( ( ( X ) + ( ( Y ) -1 ) ) & ~( ( Y ) -1 ) )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	Min

    @abstract	Returns the lesser of X and Y.
 */

#if ( !defined( Min ) )
    #define Min( X, Y )     ( ( ( X ) < ( Y ) ) ? ( X ) : ( Y ) )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	Max

    @abstract	Returns the greater of X and Y.
 */

#if ( !defined( Max ) )
    #define Max( X, Y )     ( ( ( X ) > ( Y ) ) ? ( X ) : ( Y ) )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	InsertBits

    @abstract	Inserts BITS (both 0 and 1 bits) into X, controlled by MASK and SHIFT, and returns the result.

    @discussion

    MASK is the bitmask of the bits in the final position.
    SHIFT is the number of bits to shift left for 1 to reach the first bit position of MASK.

    For example, if you wanted to insert 0x3 into the leftmost 4 bits of a 32-bit value:

    InsertBits( 0, 0x3, 0xF0000000U, 28 ) == 0x30000000
 */

#define InsertBits( X, BITS, MASK, SHIFT )      ( ( ( X ) &~( MASK ) ) | ( ( ( BITS ) << ( SHIFT ) ) & ( MASK ) ) )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	ExtractBits

    @abstract	Extracts bits from X, controlled by MASK and SHIFT, and returns the result.

    @discussion

    MASK is the bitmask of the bits in the final position.
    SHIFT is the number of bits to shift right to right justify MASK.

    For example, if you had a 32-bit value (e.g. 0x30000000) wanted the left-most 4 bits (e.g. 3 in this example):

    ExtractBits( 0x30000000U, 0xF0000000U, 28 ) == 0x3
 */

#define ExtractBits( X, MASK, SHIFT )           ( ( ( X ) >> ( SHIFT ) ) & ( ( MASK ) >> ( SHIFT ) ) )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	Stringify

    @abstract	Stringify's an expression.

    @discussion

    Stringify macros to process raw text passed via -D options to C string constants. The double-wrapping is necessary
    because the C preprocessor doesn't perform its normal argument expansion pre-scan with stringified macros so the
    -D macro needs to be expanded once via the wrapper macro then stringified so the raw text is stringified. Otherwise,
    the replacement value would be used instead of the symbolic name (only for preprocessor symbols like #defines).

    For example:

 #define	kMyConstant		1

        printf( "%s", Stringify( kMyConstant ) );			// Prints "kMyConstant"
        printf( "%s", StringifyExpansion( kMyConstant ) );	// Prints "1"

    Non-preprocessor symbols do not have this issue. For example:

        enum
        {
            kMyConstant = 1
        };

        printf( "%s", Stringify( kMyConstant ) );			// Prints "kMyConstant"
        printf( "%s", StringifyExpansion( kMyConstant ) );	// Prints "kMyConstant"

    See <http://gcc.gnu.org/onlinedocs/cpp/Argument-Prescan.html> for more info on C preprocessor pre-scanning.
 */

#define Stringify( X )              # X
#define StringifyExpansion( X )     Stringify( X )

// should we include padding here?
#define endof_field(_type, _field) (offsetof(_type, _field) + sizeof_field(_type, _field))

// not sure what this is supposed to be
#define STATIC_PARAM

// these are used in mDNSResponder
#if __has_feature(objc_arc)
    #define arc_safe_super_dealloc()
    #define arc_safe_autorelease(_expr) (_expr)
#else
    #define arc_safe_super_dealloc() [super dealloc]
    #define arc_safe_autorelease(_expr) [(_expr) autorelease]
#endif

#if ( !defined( __cplusplus ) && !__bool_true_false_are_defined )
    #define COMMON_SERVICES_NEEDS_BOOL      1
#else
    #define COMMON_SERVICES_NEEDS_BOOL      0
#endif

#if ( COMMON_SERVICES_NEEDS_BOOL )
    typedef int bool;

    #define bool bool

    #if ( !defined( __MACTYPES__ ) && !defined( true ) && !defined( false ) )
        #define true    1
        #define false   0
    #endif

    #define __bool_true_false_are_defined       1
#endif

#if 0
#pragma mark == Errors ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@enum		OSStatus

    @abstract	Status Code

    @constant	kNoErr						    0 No error occurred.
    @constant	kInProgressErr				    1 Operation in progress.
    @constant	kUnknownErr					-6700 Unknown error occurred.
    @constant	kOptionErr					-6701 Option was not acceptable.
    @constant	kSelectorErr				-6702 Selector passed in is invalid or unknown.
    @constant	kExecutionStateErr			-6703 Call made in the wrong execution state (e.g. called at interrupt time).
    @constant	kPathErr					-6704 Path is invalid, too long, or otherwise not usable.
    @constant	kParamErr					-6705 Parameter is incorrect, missing, or not appropriate.
    @constant	kParamCountErr				-6706 Incorrect or unsupported number of parameters.
    @constant	kCommandErr					-6707 Command invalid or not supported.
    @constant	kIDErr						-6708 Unknown, invalid, or inappropriate identifier.
    @constant	kStateErr					-6709 Not in appropriate state to perform operation.
    @constant	kRangeErr					-6710 Index is out of range or not valid.
    @constant	kRequestErr					-6711 Request was improperly formed or not appropriate.
    @constant	kResponseErr				-6712 Response was incorrect or out of sequence.
    @constant	kChecksumErr				-6713 Checksum does not match the actual data.
    @constant	kNotHandledErr				-6714 Operation was not handled (or not handled completely).
    @constant	kVersionErr					-6715 Version is not incorrect or not compatibile.
    @constant	kSignatureErr				-6716 Signature did not match what was expected.
    @constant	kFormatErr					-6717 Unknown, invalid, or inappropriate file/data format.
    @constant	kNotInitializedErr			-6718 Action request before needed services were initialized.
    @constant	kAlreadyInitializedErr		-6719 Attempt made to initialize when already initialized.
    @constant	kNotInUseErr				-6720 Object not in use (e.g. cannot abort if not already in use).
    @constant	kInUseErr					-6721 Object is in use (e.g. cannot reuse active param blocks).
    @constant	kTimeoutErr					-6722 Timeout occurred.
    @constant	kCanceledErr				-6723 Operation canceled (successful cancel).
    @constant	kAlreadyCanceledErr			-6724 Operation has already been canceled.
    @constant	kCannotCancelErr			-6725 Operation could not be canceled (maybe already done or invalid).
    @constant	kDeletedErr					-6726 Object has already been deleted.
    @constant	kNotFoundErr				-6727 Something was not found.
    @constant	kNoMemoryErr				-6728 Not enough memory was available to perform the operation.
    @constant	kNoResourcesErr				-6729 Resources unavailable to perform the operation.
    @constant	kDuplicateErr				-6730 Duplicate found or something is a duplicate.
    @constant	kImmutableErr				-6731 Entity is not changeable.
    @constant	kUnsupportedDataErr			-6732 Data is unknown or not supported.
    @constant	kIntegrityErr				-6733 Data is corrupt.
    @constant	kIncompatibleErr			-6734 Data is not compatible or it is in an incompatible format.
    @constant	kUnsupportedErr				-6735 Feature or option is not supported.
    @constant	kUnexpectedErr				-6736 Error occurred that was not expected.
    @constant	kValueErr					-6737 Value is not appropriate.
    @constant	kNotReadableErr				-6738 Could not read or reading is not allowed.
    @constant	kNotWritableErr				-6739 Could not write or writing is not allowed.
    @constant	kBadReferenceErr			-6740 An invalid or inappropriate reference was specified.
    @constant	kFlagErr					-6741 An invalid, inappropriate, or unsupported flag was specified.
    @constant	kMalformedErr				-6742 Something was not formed correctly.
    @constant	kSizeErr					-6743 Size was too big, too small, or not appropriate.
    @constant	kNameErr					-6744 Name was not correct, allowed, or appropriate.
    @constant	kNotReadyErr				-6745 Device or service is not ready.
    @constant	kReadErr					-6746 Could not read.
    @constant	kWriteErr					-6747 Could not write.
    @constant	kMismatchErr				-6748 Something does not match.
    @constant	kDateErr					-6749 Date is invalid or out-of-range.
    @constant	kUnderrunErr				-6750 Less data than expected.
    @constant	kOverrunErr					-6751 More data than expected.
    @constant	kEndingErr					-6752 Connection, session, or something is ending.
    @constant	kConnectionErr				-6753 Connection failed or could not be established.
    @constant	kAuthenticationErr			-6754 Authentication failed or is not supported.
    @constant	kOpenErr					-6755 Could not open file, pipe, device, etc.
    @constant	kTypeErr					-6756 Incorrect or incompatible type (e.g. file, data, etc.).
    @constant	kSkipErr					-6757 Items should be or was skipped.
    @constant	kNoAckErr					-6758 No acknowledge.
    @constant	kCollisionErr				-6759 Collision occurred (e.g. two on bus at same time).
    @constant	kBackoffErr					-6760 Backoff in progress and operation intentionally failed.
    @constant	kNoAddressAckErr			-6761 No acknowledge of address.
    @constant	kBusyErr					-6762 Cannot perform because something is busy.
    @constant	kNoSpaceErr					-6763 Not enough space to perform operation.
 */

#if ( !TARGET_OS_MAC && !TARGET_API_MAC_OSX_KERNEL )
typedef int32_t OSStatus;
#endif

#define kNoErr                      0
#define kInProgressErr              1

// Generic error codes are in the range -6700 to -6779.

#define kGenericErrorBase           -6700   // Starting error code for all generic errors.

#define kUnknownErr                 -6700
#define kOptionErr                  -6701
#define kSelectorErr                -6702
#define kExecutionStateErr          -6703
#define kPathErr                    -6704
#define kParamErr                   -6705
#define kParamCountErr              -6706
#define kCommandErr                 -6707
#define kIDErr                      -6708
#define kStateErr                   -6709
#define kRangeErr                   -6710
#define kRequestErr                 -6711
#define kResponseErr                -6712
#define kChecksumErr                -6713
#define kNotHandledErr              -6714
#define kVersionErr                 -6715
#define kSignatureErr               -6716
#define kFormatErr                  -6717
#define kNotInitializedErr          -6718
#define kAlreadyInitializedErr      -6719
#define kNotInUseErr                -6720
#define kInUseErr                   -6721
#define kTimeoutErr                 -6722
#define kCanceledErr                -6723
#define kAlreadyCanceledErr         -6724
#define kCannotCancelErr            -6725
#define kDeletedErr                 -6726
#define kNotFoundErr                -6727
#define kNoMemoryErr                -6728
#define kNoResourcesErr             -6729
#define kDuplicateErr               -6730
#define kImmutableErr               -6731
#define kUnsupportedDataErr         -6732
#define kIntegrityErr               -6733
#define kIncompatibleErr            -6734
#define kUnsupportedErr             -6735
#define kUnexpectedErr              -6736
#define kValueErr                   -6737
#define kNotReadableErr             -6738
#define kNotWritableErr             -6739
#define kBadReferenceErr            -6740
#define kFlagErr                    -6741
#define kMalformedErr               -6742
#define kSizeErr                    -6743
#define kNameErr                    -6744
#define kNotReadyErr                -6745
#define kReadErr                    -6746
#define kWriteErr                   -6747
#define kMismatchErr                -6748
#define kDateErr                    -6749
#define kUnderrunErr                -6750
#define kOverrunErr                 -6751
#define kEndingErr                  -6752
#define kConnectionErr              -6753
#define kAuthenticationErr          -6754
#define kOpenErr                    -6755
#define kTypeErr                    -6756
#define kSkipErr                    -6757
#define kNoAckErr                   -6758
#define kCollisionErr               -6759
#define kBackoffErr                 -6760
#define kNoAddressAckErr            -6761
#define kBusyErr                    -6762
#define kNoSpaceErr                 -6763

#define kGenericErrorEnd            -6779   // Last generic error code (inclusive)

#if 0
#pragma mark == Mac Compatibility ==
#endif

//===========================================================================================================================
//	Mac Compatibility
//===========================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------
/*!	@enum		Duration

    @abstract	Type used to specify a duration of time.

    @constant	kDurationImmediate			Indicates no delay/wait time.
    @constant	kDurationMicrosecond		Microsecond units.
    @constant	kDurationMillisecond		Millisecond units.
    @constant	kDurationSecond				Second units.
    @constant	kDurationMinute				Minute units.
    @constant	kDurationHour				Hour units.
    @constant	kDurationDay				Day units.
    @constant	kDurationForever			Infinite period of time (no timeout).

    @discussion

    Duration values are intended to be multiplied by the specific interval to achieve an actual duration. For example,
    to wait for 5 seconds you would use "5 * kDurationSecond".
 */

#define kDurationImmediate              0L
#define kDurationMicrosecond            -1L
#define kDurationMillisecond            1L
#define kDurationSecond                 ( 1000L * kDurationMillisecond )
#define kDurationMinute                 ( 60L * kDurationSecond )
#define kDurationHour                   ( 60L * kDurationMinute )
#define kDurationDay                    ( 24L * kDurationHour )
#define kDurationForever                0x7FFFFFFFL

// Seconds <-> Minutes <-> Hours <-> Days <-> Weeks <-> Months <-> Years conversions

#define kNanosecondsPerMicrosecond      1000
#define kNanosecondsPerMillisecond      1000000
#define kNanosecondsPerSecond           1000000000
#define kMicrosecondsPerSecond          1000000
#define kMicrosecondsPerMillisecond     1000
#define kMillisecondsPerSecond          1000
#define kSecondsPerMinute               60
#define kSecondsPerHour                 ( 60 * 60 )             // 3600
#define kSecondsPerDay                  ( 60 * 60 * 24 )        // 86400
#define kSecondsPerWeek                 ( 60 * 60 * 24 * 7 )    // 604800
#define kMinutesPerHour                 60
#define kMinutesPerDay                  ( 60 * 24 )             // 1440
#define kHoursPerDay                    24
#define kDaysPerWeek                    7
#define kWeeksPerYear                   52
#define kMonthsPerYear                  12

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	VersionStages

    @abstract	NumVersion-style version stages.
 */

#define kVersionStageDevelopment        0x20
#define kVersionStageAlpha              0x40
#define kVersionStageBeta               0x60
#define kVersionStageFinal              0x80

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	NumVersionBuild

    @abstract	Builds a 32-bit Mac-style NumVersion value (e.g. NumVersionBuild( 1, 2, 3, kVersionStageBeta, 4 ) -> 1.2.3b4).
 */

#define NumVersionBuild( MAJOR, MINOR, BUGFIX, STAGE, REV ) \
    ( ( ( ( MAJOR )  & 0xFF ) << 24 ) |                     \
      ( ( ( MINOR )  & 0x0F ) << 20 ) |                     \
      ( ( ( BUGFIX ) & 0x0F ) << 16 ) |                     \
      ( ( ( STAGE )  & 0xFF ) <<  8 ) |                     \
      ( ( ( REV )    & 0xFF )       ) )

#define NumVersionExtractMajor( VERSION )               ( (uint8_t)( ( ( VERSION ) >> 24 ) & 0xFF ) )
#define NumVersionExtractMinorAndBugFix( VERSION )      ( (uint8_t)( ( ( VERSION ) >> 16 ) & 0xFF ) )
#define NumVersionExtractMinor( VERSION )               ( (uint8_t)( ( ( VERSION ) >> 20 ) & 0x0F ) )
#define NumVersionExtractBugFix( VERSION )              ( (uint8_t)( ( ( VERSION ) >> 16 ) & 0x0F ) )
#define NumVersionExtractStage( VERSION )               ( (uint8_t)( ( ( VERSION ) >>  8 ) & 0xFF ) )
#define NumVersionExtractRevision( VERSION )            ( (uint8_t)(   ( VERSION )         & 0xFF ) )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@function	NumVersionCompare

    @abstract	Compares two NumVersion values and returns the following values:

        left < right -> -1
        left > right ->  1
        left = right ->  0
 */

int NumVersionCompare( uint32_t inLeft, uint32_t inRight );

#if 0
#pragma mark == Binary Constants ==
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	binary_4

    @abstract	Macro to generate an 4-bit constant using binary notation (e.g. binary_4( 1010 ) == 0xA).
 */

#define binary_4( a )                       binary_4_hex_wrap( hex_digit4( a ) )
#define binary_4_hex_wrap( a )              binary_4_hex( a )
#define binary_4_hex( a )                   ( 0x ## a )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	binary_8

    @abstract	Macro to generate an 8-bit constant using binary notation (e.g. binary_8( 01111011 ) == 0x7B).
 */

#define binary_8( a )                       binary_8_hex_wrap( hex_digit8( a ) )
#define binary_8_hex_wrap( a )              binary_8_hex( a )
#define binary_8_hex( a )                   ( 0x ## a )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	binary_16

    @abstract	Macro to generate an 16-bit constant using binary notation (e.g. binary_16( 01111011, 01111011 ) == 0x7B7B).
 */

#define binary_16( a, b )                   binary_16_hex_wrap( hex_digit8( a ), hex_digit8( b ) )
#define binary_16_hex_wrap( a, b )          binary_16_hex( a, b )
#define binary_16_hex( a, b )               ( 0x ## a ## b )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@defined	binary_32

    @abstract	Macro to generate an 32-bit constant using binary notation
                (e.g. binary_32( 01111011, 01111011, 01111011, 01111011 ) == 0x7B7B7B7B).
 */

#define binary_32( a, b, c, d )             binary_32_hex_wrap( hex_digit8( a ), hex_digit8( b ), hex_digit8( c ), hex_digit8( d ) )
#define binary_32_hex_wrap( a, b, c, d )    binary_32_hex( a, b, c, d )
#define binary_32_hex( a, b, c, d )         ( 0x ## a ## b ## c ## d )

// Binary Constant Helpers

#define hex_digit8( a )                     HEX_DIGIT_ ## a
#define hex_digit4( a )                     HEX_DIGIT_ ## 0000 ## a

#define HEX_DIGIT_00000000                  00
#define HEX_DIGIT_00000001                  01
#define HEX_DIGIT_00000010                  02
#define HEX_DIGIT_00000011                  03
#define HEX_DIGIT_00000100                  04
#define HEX_DIGIT_00000101                  05
#define HEX_DIGIT_00000110                  06
#define HEX_DIGIT_00000111                  07
#define HEX_DIGIT_00001000                  08
#define HEX_DIGIT_00001001                  09
#define HEX_DIGIT_00001010                  0A
#define HEX_DIGIT_00001011                  0B
#define HEX_DIGIT_00001100                  0C
#define HEX_DIGIT_00001101                  0D
#define HEX_DIGIT_00001110                  0E
#define HEX_DIGIT_00001111                  0F
#define HEX_DIGIT_00010000                  10
#define HEX_DIGIT_00010001                  11
#define HEX_DIGIT_00010010                  12
#define HEX_DIGIT_00010011                  13
#define HEX_DIGIT_00010100                  14
#define HEX_DIGIT_00010101                  15
#define HEX_DIGIT_00010110                  16
#define HEX_DIGIT_00010111                  17
#define HEX_DIGIT_00011000                  18
#define HEX_DIGIT_00011001                  19
#define HEX_DIGIT_00011010                  1A
#define HEX_DIGIT_00011011                  1B
#define HEX_DIGIT_00011100                  1C
#define HEX_DIGIT_00011101                  1D
#define HEX_DIGIT_00011110                  1E
#define HEX_DIGIT_00011111                  1F
#define HEX_DIGIT_00100000                  20
#define HEX_DIGIT_00100001                  21
#define HEX_DIGIT_00100010                  22
#define HEX_DIGIT_00100011                  23
#define HEX_DIGIT_00100100                  24
#define HEX_DIGIT_00100101                  25
#define HEX_DIGIT_00100110                  26
#define HEX_DIGIT_00100111                  27
#define HEX_DIGIT_00101000                  28
#define HEX_DIGIT_00101001                  29
#define HEX_DIGIT_00101010                  2A
#define HEX_DIGIT_00101011                  2B
#define HEX_DIGIT_00101100                  2C
#define HEX_DIGIT_00101101                  2D
#define HEX_DIGIT_00101110                  2E
#define HEX_DIGIT_00101111                  2F
#define HEX_DIGIT_00110000                  30
#define HEX_DIGIT_00110001                  31
#define HEX_DIGIT_00110010                  32
#define HEX_DIGIT_00110011                  33
#define HEX_DIGIT_00110100                  34
#define HEX_DIGIT_00110101                  35
#define HEX_DIGIT_00110110                  36
#define HEX_DIGIT_00110111                  37
#define HEX_DIGIT_00111000                  38
#define HEX_DIGIT_00111001                  39
#define HEX_DIGIT_00111010                  3A
#define HEX_DIGIT_00111011                  3B
#define HEX_DIGIT_00111100                  3C
#define HEX_DIGIT_00111101                  3D
#define HEX_DIGIT_00111110                  3E
#define HEX_DIGIT_00111111                  3F
#define HEX_DIGIT_01000000                  40
#define HEX_DIGIT_01000001                  41
#define HEX_DIGIT_01000010                  42
#define HEX_DIGIT_01000011                  43
#define HEX_DIGIT_01000100                  44
#define HEX_DIGIT_01000101                  45
#define HEX_DIGIT_01000110                  46
#define HEX_DIGIT_01000111                  47
#define HEX_DIGIT_01001000                  48
#define HEX_DIGIT_01001001                  49
#define HEX_DIGIT_01001010                  4A
#define HEX_DIGIT_01001011                  4B
#define HEX_DIGIT_01001100                  4C
#define HEX_DIGIT_01001101                  4D
#define HEX_DIGIT_01001110                  4E
#define HEX_DIGIT_01001111                  4F
#define HEX_DIGIT_01010000                  50
#define HEX_DIGIT_01010001                  51
#define HEX_DIGIT_01010010                  52
#define HEX_DIGIT_01010011                  53
#define HEX_DIGIT_01010100                  54
#define HEX_DIGIT_01010101                  55
#define HEX_DIGIT_01010110                  56
#define HEX_DIGIT_01010111                  57
#define HEX_DIGIT_01011000                  58
#define HEX_DIGIT_01011001                  59
#define HEX_DIGIT_01011010                  5A
#define HEX_DIGIT_01011011                  5B
#define HEX_DIGIT_01011100                  5C
#define HEX_DIGIT_01011101                  5D
#define HEX_DIGIT_01011110                  5E
#define HEX_DIGIT_01011111                  5F
#define HEX_DIGIT_01100000                  60
#define HEX_DIGIT_01100001                  61
#define HEX_DIGIT_01100010                  62
#define HEX_DIGIT_01100011                  63
#define HEX_DIGIT_01100100                  64
#define HEX_DIGIT_01100101                  65
#define HEX_DIGIT_01100110                  66
#define HEX_DIGIT_01100111                  67
#define HEX_DIGIT_01101000                  68
#define HEX_DIGIT_01101001                  69
#define HEX_DIGIT_01101010                  6A
#define HEX_DIGIT_01101011                  6B
#define HEX_DIGIT_01101100                  6C
#define HEX_DIGIT_01101101                  6D
#define HEX_DIGIT_01101110                  6E
#define HEX_DIGIT_01101111                  6F
#define HEX_DIGIT_01110000                  70
#define HEX_DIGIT_01110001                  71
#define HEX_DIGIT_01110010                  72
#define HEX_DIGIT_01110011                  73
#define HEX_DIGIT_01110100                  74
#define HEX_DIGIT_01110101                  75
#define HEX_DIGIT_01110110                  76
#define HEX_DIGIT_01110111                  77
#define HEX_DIGIT_01111000                  78
#define HEX_DIGIT_01111001                  79
#define HEX_DIGIT_01111010                  7A
#define HEX_DIGIT_01111011                  7B
#define HEX_DIGIT_01111100                  7C
#define HEX_DIGIT_01111101                  7D
#define HEX_DIGIT_01111110                  7E
#define HEX_DIGIT_01111111                  7F
#define HEX_DIGIT_10000000                  80
#define HEX_DIGIT_10000001                  81
#define HEX_DIGIT_10000010                  82
#define HEX_DIGIT_10000011                  83
#define HEX_DIGIT_10000100                  84
#define HEX_DIGIT_10000101                  85
#define HEX_DIGIT_10000110                  86
#define HEX_DIGIT_10000111                  87
#define HEX_DIGIT_10001000                  88
#define HEX_DIGIT_10001001                  89
#define HEX_DIGIT_10001010                  8A
#define HEX_DIGIT_10001011                  8B
#define HEX_DIGIT_10001100                  8C
#define HEX_DIGIT_10001101                  8D
#define HEX_DIGIT_10001110                  8E
#define HEX_DIGIT_10001111                  8F
#define HEX_DIGIT_10010000                  90
#define HEX_DIGIT_10010001                  91
#define HEX_DIGIT_10010010                  92
#define HEX_DIGIT_10010011                  93
#define HEX_DIGIT_10010100                  94
#define HEX_DIGIT_10010101                  95
#define HEX_DIGIT_10010110                  96
#define HEX_DIGIT_10010111                  97
#define HEX_DIGIT_10011000                  98
#define HEX_DIGIT_10011001                  99
#define HEX_DIGIT_10011010                  9A
#define HEX_DIGIT_10011011                  9B
#define HEX_DIGIT_10011100                  9C
#define HEX_DIGIT_10011101                  9D
#define HEX_DIGIT_10011110                  9E
#define HEX_DIGIT_10011111                  9F
#define HEX_DIGIT_10100000                  A0
#define HEX_DIGIT_10100001                  A1
#define HEX_DIGIT_10100010                  A2
#define HEX_DIGIT_10100011                  A3
#define HEX_DIGIT_10100100                  A4
#define HEX_DIGIT_10100101                  A5
#define HEX_DIGIT_10100110                  A6
#define HEX_DIGIT_10100111                  A7
#define HEX_DIGIT_10101000                  A8
#define HEX_DIGIT_10101001                  A9
#define HEX_DIGIT_10101010                  AA
#define HEX_DIGIT_10101011                  AB
#define HEX_DIGIT_10101100                  AC
#define HEX_DIGIT_10101101                  AD
#define HEX_DIGIT_10101110                  AE
#define HEX_DIGIT_10101111                  AF
#define HEX_DIGIT_10110000                  B0
#define HEX_DIGIT_10110001                  B1
#define HEX_DIGIT_10110010                  B2
#define HEX_DIGIT_10110011                  B3
#define HEX_DIGIT_10110100                  B4
#define HEX_DIGIT_10110101                  B5
#define HEX_DIGIT_10110110                  B6
#define HEX_DIGIT_10110111                  B7
#define HEX_DIGIT_10111000                  B8
#define HEX_DIGIT_10111001                  B9
#define HEX_DIGIT_10111010                  BA
#define HEX_DIGIT_10111011                  BB
#define HEX_DIGIT_10111100                  BC
#define HEX_DIGIT_10111101                  BD
#define HEX_DIGIT_10111110                  BE
#define HEX_DIGIT_10111111                  BF
#define HEX_DIGIT_11000000                  C0
#define HEX_DIGIT_11000001                  C1
#define HEX_DIGIT_11000010                  C2
#define HEX_DIGIT_11000011                  C3
#define HEX_DIGIT_11000100                  C4
#define HEX_DIGIT_11000101                  C5
#define HEX_DIGIT_11000110                  C6
#define HEX_DIGIT_11000111                  C7
#define HEX_DIGIT_11001000                  C8
#define HEX_DIGIT_11001001                  C9
#define HEX_DIGIT_11001010                  CA
#define HEX_DIGIT_11001011                  CB
#define HEX_DIGIT_11001100                  CC
#define HEX_DIGIT_11001101                  CD
#define HEX_DIGIT_11001110                  CE
#define HEX_DIGIT_11001111                  CF
#define HEX_DIGIT_11010000                  D0
#define HEX_DIGIT_11010001                  D1
#define HEX_DIGIT_11010010                  D2
#define HEX_DIGIT_11010011                  D3
#define HEX_DIGIT_11010100                  D4
#define HEX_DIGIT_11010101                  D5
#define HEX_DIGIT_11010110                  D6
#define HEX_DIGIT_11010111                  D7
#define HEX_DIGIT_11011000                  D8
#define HEX_DIGIT_11011001                  D9
#define HEX_DIGIT_11011010                  DA
#define HEX_DIGIT_11011011                  DB
#define HEX_DIGIT_11011100                  DC
#define HEX_DIGIT_11011101                  DD
#define HEX_DIGIT_11011110                  DE
#define HEX_DIGIT_11011111                  DF
#define HEX_DIGIT_11100000                  E0
#define HEX_DIGIT_11100001                  E1
#define HEX_DIGIT_11100010                  E2
#define HEX_DIGIT_11100011                  E3
#define HEX_DIGIT_11100100                  E4
#define HEX_DIGIT_11100101                  E5
#define HEX_DIGIT_11100110                  E6
#define HEX_DIGIT_11100111                  E7
#define HEX_DIGIT_11101000                  E8
#define HEX_DIGIT_11101001                  E9
#define HEX_DIGIT_11101010                  EA
#define HEX_DIGIT_11101011                  EB
#define HEX_DIGIT_11101100                  EC
#define HEX_DIGIT_11101101                  ED
#define HEX_DIGIT_11101110                  EE
#define HEX_DIGIT_11101111                  EF
#define HEX_DIGIT_11110000                  F0
#define HEX_DIGIT_11110001                  F1
#define HEX_DIGIT_11110010                  F2
#define HEX_DIGIT_11110011                  F3
#define HEX_DIGIT_11110100                  F4
#define HEX_DIGIT_11110101                  F5
#define HEX_DIGIT_11110110                  F6
#define HEX_DIGIT_11110111                  F7
#define HEX_DIGIT_11111000                  F8
#define HEX_DIGIT_11111001                  F9
#define HEX_DIGIT_11111010                  FA
#define HEX_DIGIT_11111011                  FB
#define HEX_DIGIT_11111100                  FC
#define HEX_DIGIT_11111101                  FD
#define HEX_DIGIT_11111110                  FE
#define HEX_DIGIT_11111111                  FF

#ifdef  __cplusplus
}
#endif

#endif  // __COMMON_SERVICES__
