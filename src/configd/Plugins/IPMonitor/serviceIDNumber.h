/*
 * Copyright (c) 2019 Apple Inc.  All Rights Reserved.
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

/*
 * serviceID_number.h
 * - assigns numbers to serviceID strings
 */

#ifndef	_SERVICEID_NUMBER_H
#define	_SERVICEID_NUMBER_H

#include <CoreFoundation/CFString.h>
#include <sys/types.h>

/*
 * Type: serviceIDNumber
 * Purpose:
 *   A compact way to represent a serviceID that doesn't involve needing
 *   to worry about memory allocation or large comparisions.
 */
typedef u_long		serviceIDNumber, *serviceIDNumberRef;

/*
 * Const: kserviceIDNumberZero
 *
 * Purpose:
 *   The zero value for serviceIDNumber. It will never be returned,
 *   it is a reserved value.
 */
#define kserviceIDNumberZero	(serviceIDNumber)(0)

/*
 * Function: serviceIDNumberGetIfPresent
 *
 * Purpose:
 *   Returns TRUE and the serviceIDNumber for the given serviceID, if it
 *   is present, FALSE otherwise.
 */
Boolean
serviceIDNumberGetIfPresent(CFStringRef serviceID, serviceIDNumberRef sidn);

/*
 * Function: serviceIDNumberGet
 *
 * Purpose:
 *   Returns a unique serviceIDNumber for the given serviceID.
 */
serviceIDNumber
serviceIDNumberGet(CFStringRef serviceID);

/*
 * Function: serviceIDNumberRemove
 *
 * Purpose:
 *   Removes the assigned serviceIDNumber for the given serviceID.
 */
void
serviceIDNumberRemove(CFStringRef serviceID);


/*
 * Function: serviceIDNumberInit
 *
 * Purpose:
 *   Initialize the serviceID number database.
 */
void
serviceIDNumberInit(void);

#endif /* _SERVICEID_NUMBER_H */
