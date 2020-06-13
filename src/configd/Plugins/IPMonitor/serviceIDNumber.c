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
 * serviceID_number.c
 * - assigns numbers to serviceID strings
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "serviceIDNumber.h"
#include <CoreFoundation/CFDictionary.h>

#if TEST_SERVICEID_NUMBER
#include <SystemConfiguration/SCPrivate.h>
#endif /* TEST_SERVICEID_NUMBER */


/* dictionary to hold serviceIDNumber: key is the serviceID */
static CFMutableDictionaryRef	S_serviceID_to_number_dict;

/* dictionary to hold serviceID: key is the serviceIDNumber */
static CFMutableDictionaryRef	S_number_to_serviceID_dict;

static Boolean
serviceIDNumberEqual(const void * ptr1, const void * ptr2)
{
	return (((serviceIDNumber)ptr1) == ((serviceIDNumber)ptr2));
}

static CFStringRef
serviceIDNumberCopyDescription(const void * ptr)
{
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("%ld"),
					(serviceIDNumber)ptr);
}

static CFHashCode
serviceIDNumberHash(const void * ptr) {
	return (CFHashCode)((serviceIDNumber)ptr);
}

static const CFDictionaryValueCallBacks kserviceIDNumberValueCallBacks = {
	0, NULL, NULL, serviceIDNumberCopyDescription, serviceIDNumberEqual
};

static const CFDictionaryKeyCallBacks kserviceIDNumberKeyCallBacks = {
	0, NULL, NULL, serviceIDNumberCopyDescription, serviceIDNumberEqual,
	serviceIDNumberHash
};

/**
 ** S_serviceID_numbers
 **/

__private_extern__ Boolean
serviceIDNumberGetIfPresent(CFStringRef serviceID, serviceIDNumber *sidn)
{
	Boolean			has_number;
	const void *		val;

	has_number = CFDictionaryGetValueIfPresent(S_serviceID_to_number_dict,
						   serviceID, &val);
	if (has_number) {
		*sidn = (serviceIDNumber)val;
	}
	return (has_number);
}

/*
 * Function: serviceIDNumberGet
 * Purpose:
 *   Return the currently assigned serviceIDNumber for the given serviceID.
 *   If one is already assigned, return that. If one isn't assigned, check
 *   the next integer value after 'current_sidn', but skip zero.
 *   If that number is assigned, pick the next one.
 */
static serviceIDNumber	S_current_sidn;

__private_extern__ serviceIDNumber
serviceIDNumberGet(CFStringRef serviceID)
{
	serviceIDNumber		sidn;

	if (serviceIDNumberGetIfPresent(serviceID, &sidn)) {
		return (sidn);
	}
	while (1) {
		/* assign a number to the serviceID */
		S_current_sidn++;
		if (S_current_sidn == kserviceIDNumberZero) {
			/* skip zero */
			S_current_sidn++;
		}
		/* if it's in use, skip to the next value */
		if (CFDictionaryContainsKey(S_number_to_serviceID_dict,
					    (const void *)S_current_sidn)) {
			continue;
		}
		/* it's not in use, use it */
		sidn = S_current_sidn;
		CFDictionarySetValue(S_serviceID_to_number_dict,
				     serviceID, (const void *)sidn);
		CFDictionarySetValue(S_number_to_serviceID_dict,
				     (const void *)sidn, serviceID);
		break;
	}
	return (sidn);
}


__private_extern__ void
serviceIDNumberRemove(CFStringRef serviceID)
{
	const void *	val;

	if (CFDictionaryGetValueIfPresent(S_serviceID_to_number_dict, serviceID,
					  &val)) {
#if TEST_SERVICEID_NUMBER
		SCPrint(TRUE, stdout, CFSTR("Removing %@ %ld\n"),
			serviceID, (serviceIDNumber)val);
#endif
		CFDictionaryRemoveValue(S_serviceID_to_number_dict, serviceID);
		CFDictionaryRemoveValue(S_number_to_serviceID_dict, val);
	}
}

__private_extern__ void
serviceIDNumberInit(void)
{
	S_serviceID_to_number_dict
		= CFDictionaryCreateMutable(NULL, 0,
					    &kCFTypeDictionaryKeyCallBacks,
					    &kserviceIDNumberValueCallBacks);
	S_number_to_serviceID_dict
		= CFDictionaryCreateMutable(NULL, 0,
					    &kserviceIDNumberKeyCallBacks,
					    &kCFTypeDictionaryValueCallBacks);
}

#if TEST_SERVICEID_NUMBER

static CFStringRef
my_CFUUIDStringCreate(CFAllocatorRef alloc)
{
	CFUUIDRef 	uuid;
	CFStringRef	uuid_str;

	uuid = CFUUIDCreate(alloc);
	uuid_str = CFUUIDCreateString(alloc, uuid);
	CFRelease(uuid);
	return (uuid_str);
}

int
main()
{
#define N_LIST		10
	CFStringRef	serviceID_list[N_LIST];

	serviceIDNumberInit();
	for (int i = 0; i < N_LIST; i++) {
		CFStringRef	serviceID = my_CFUUIDStringCreate(NULL);
		serviceIDNumber	sidn;

		/* force a collision */
		S_current_sidn = -1;

		sidn = serviceIDNumberGet(serviceID);
		SCPrint(TRUE, stdout, CFSTR("%d: %@ %ld\n"),
			i, serviceID, sidn);
		serviceID_list[i] = serviceID;

	}
	for (int i = 0; i < N_LIST; i++) {
		CFStringRef	serviceID = serviceID_list[i];
		serviceIDNumber	sidn;

		if (!serviceIDNumberGetIfPresent(serviceID, &sidn)) {
			SCPrint(TRUE, stderr, CFSTR("Failed to find %@\n"),
				serviceID);
			exit(1);
		}
		SCPrint(TRUE, stdout, CFSTR("%@ => %ld\n"), serviceID, sidn);
	}
	{
		serviceIDNumber	sidn;

		if (serviceIDNumberGetIfPresent(CFSTR("blah"), &sidn)) {
			fprintf(stderr,
				"Shouldn't have been able to look that up\n");
			exit(1);
		}
	}

	for (int i = 0; i < N_LIST / 2; i++) {
		CFStringRef	serviceID = serviceID_list[i];
		serviceIDNumber	sidn;

		serviceIDNumberRemove(serviceID);
		if (serviceIDNumberGetIfPresent(serviceID, &sidn)) {
			SCPrint(TRUE, stderr,
				CFSTR("Found %@, but shouldn't have\n"),
				serviceID);
			exit(1);
		}
	}

	for (int i = 0; i < N_LIST; i++) {
		CFStringRef	serviceID = serviceID_list[i];
		serviceIDNumber	sidn;

		sidn = serviceIDNumberGet(serviceID);
		SCPrint(TRUE, stdout, CFSTR("%d: %@ %ld\n"),
			i, serviceID, sidn);
	}
	exit(0);
}
#endif /* TEST_SERVICEID_NUMBER */
