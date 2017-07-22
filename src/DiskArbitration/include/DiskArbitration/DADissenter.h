/*
 * Copyright (c) 1998-2015 Apple Inc. All rights reserved.
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

#ifndef __DISKARBITRATION_DADISSENTER__
#define __DISKARBITRATION_DADISSENTER__

#include <mach/error.h>
#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

#define err_local_diskarbitration err_sub( 0x368 )

/*!
 * @enum       DAReturn
 * @abstract   A return code.
 */

enum
{
    kDAReturnSuccess         = 0,
    kDAReturnError           = err_local | err_local_diskarbitration | 0x01, /* ( 0xF8DA0001 ) */
    kDAReturnBusy            = err_local | err_local_diskarbitration | 0x02, /* ( 0xF8DA0002 ) */
    kDAReturnBadArgument     = err_local | err_local_diskarbitration | 0x03, /* ( 0xF8DA0003 ) */
    kDAReturnExclusiveAccess = err_local | err_local_diskarbitration | 0x04, /* ( 0xF8DA0004 ) */
    kDAReturnNoResources     = err_local | err_local_diskarbitration | 0x05, /* ( 0xF8DA0005 ) */ 
    kDAReturnNotFound        = err_local | err_local_diskarbitration | 0x06, /* ( 0xF8DA0006 ) */
    kDAReturnNotMounted      = err_local | err_local_diskarbitration | 0x07, /* ( 0xF8DA0007 ) */
    kDAReturnNotPermitted    = err_local | err_local_diskarbitration | 0x08, /* ( 0xF8DA0008 ) */
    kDAReturnNotPrivileged   = err_local | err_local_diskarbitration | 0x09, /* ( 0xF8DA0009 ) */
    kDAReturnNotReady        = err_local | err_local_diskarbitration | 0x0A, /* ( 0xF8DA000A ) */
    kDAReturnNotWritable     = err_local | err_local_diskarbitration | 0x0B, /* ( 0xF8DA000B ) */
    kDAReturnUnsupported     = err_local | err_local_diskarbitration | 0x0C  /* ( 0xF8DA000C ) */
};

typedef mach_error_t DAReturn;

#ifndef __DISKARBITRATIOND__

/*!
 * @typedef    DADissenterRef
 * Type of a reference to DADissenter instances.
 */

typedef const struct CF_BRIDGED_TYPE( id ) __DADissenter * DADissenterRef;

/*!
 * @function   DADissenterCreate
 * @abstract   Creates a new dissenter object.
 * @param      allocator The allocator object to be used to allocate memory.
 * @param      status    The return code.
 * @param      string    The return code string.  Pass NULL for no reason.
 * @result     A reference to a new DADissenter.
 */

extern DADissenterRef DADissenterCreate( CFAllocatorRef __nullable allocator, DAReturn status, CFStringRef __nullable string );

/*!
 * @function   DADissenterGetStatus
 * @abstract   Obtains the return code.
 * @param      dissenter The DADissenter for which to obtain the return code.
 * @result     The return code.  A BSD return code, if applicable, is encoded with unix_err().
 */

extern DAReturn DADissenterGetStatus( DADissenterRef dissenter );

/*!
 * @function   DADissenterGetStatusString
 * @abstract   Obtains the return code string.
 * @param      dissenter The DADissenter for which to obtain the return code string.
 * @result     The return code string.
 */

extern CFStringRef __nullable DADissenterGetStatusString( DADissenterRef dissenter );

#endif /* !__DISKARBITRATIOND__ */

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !__DISKARBITRATION_DADISSENTER__ */
