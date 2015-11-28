/*
 * Copyright (c) 2000-2005, 2009-2011, 2013 Apple Inc. All rights reserved.
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
 * Modification History
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * November 9, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include "scutil.h"
#include "dictionary.h"


//#include <stdlib.h>
//#include <limits.h>


__private_extern__
void
do_dictInit(int argc, char **argv)
{
	if (value != NULL) {
		CFRelease(value);
	}

	value = CFDictionaryCreateMutable(NULL
					 ,0
					 ,&kCFTypeDictionaryKeyCallBacks
					 ,&kCFTypeDictionaryValueCallBacks
					 );

	return;
}


__private_extern__
void
do_dictShow(int argc, char **argv)
{
	if (value == NULL) {
		SCPrint(TRUE, stdout, CFSTR("d.show: dictionary must be initialized.\n"));
		return;
	}

	SCPrint(TRUE, stdout, CFSTR("%@\n"), value);

	return;
}


__private_extern__
void
do_dictSetKey(int argc, char **argv)
{
	CFMutableArrayRef	array		= NULL;
	Boolean			doArray		= FALSE;
	Boolean			doBoolean	= FALSE;
	Boolean			doData		= FALSE;
	Boolean			doNumeric	= FALSE;
	CFStringRef		key;
	CFMutableDictionaryRef	newValue;
	CFTypeRef		val		= NULL;

	if (value == NULL) {
		SCPrint(TRUE, stdout, CFSTR("d.add: dictionary must be initialized.\n"));
		return;
	}

	if (!isA_CFDictionary(value)) {
		SCPrint(TRUE, stdout, CFSTR("d.add: data (fetched from configuration server) is not a dictionary.\n"));
		return;
	}

	key = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	argv++; argc--;

	while (argc > 0) {
		if (strcmp(argv[0], "*") == 0) {
			/* if array requested */
			doArray = TRUE;
		} else if (strcmp(argv[0], "-") == 0) {
			/* if string values follow */
			argv++; argc--;
			break;
		} else if (strcmp(argv[0], "?") == 0) {
			/* if boolean values requested */
			doBoolean = TRUE;
		} else if (strcmp(argv[0], "%") == 0) {
			/* if [hex] data values requested */
			doData = TRUE;
		} else if (strcmp(argv[0], "#") == 0) {
			/* if numeric values requested */
			doNumeric = TRUE;
		} else {
			/* it's not a special flag */
			break;
		}
		argv++; argc--;
	}

	if (argc > 1) {
		doArray = TRUE;
	} else if (!doArray && (argc == 0)) {
		SCPrint(TRUE, stdout, CFSTR("d.add: no values.\n"));
		CFRelease(key);
		return;
	}

	if (doArray) {
		array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	while (argc > 0) {
		if (doBoolean) {
			if         ((strcasecmp(argv[0], "true") == 0) ||
				    (strcasecmp(argv[0], "t"   ) == 0) ||
				    (strcasecmp(argv[0], "yes" ) == 0) ||
				    (strcasecmp(argv[0], "y"   ) == 0) ||
				    (strcmp    (argv[0], "1"   ) == 0)) {
				val = CFRetain(kCFBooleanTrue);
			} else if ((strcasecmp(argv[0], "false") == 0) ||
				   (strcasecmp(argv[0], "f"    ) == 0) ||
				   (strcasecmp(argv[0], "no"   ) == 0) ||
				   (strcasecmp(argv[0], "n"    ) == 0) ||
				   (strcmp    (argv[0], "0"    ) == 0)) {
				val = CFRetain(kCFBooleanFalse);
			} else {
				SCPrint(TRUE, stdout, CFSTR("d.add: invalid data.\n"));
				if (doArray) CFRelease(array);
				CFRelease(key);
				return;
			}
		} else if (doData) {
			uint8_t			*bytes;
			CFMutableDataRef	data;
			int			i;
			int			j;
			int			n;

			n = (int)strlen(argv[0]);
			if ((n % 2) == 1) {
				SCPrint(TRUE, stdout, CFSTR("d.add: not enough bytes.\n"));
				if (doArray) CFRelease(array);
				CFRelease(key);
				return;
			}

			data = CFDataCreateMutable(NULL, (n / 2));
			CFDataSetLength(data, (n / 2));

			bytes = (uint8_t *)CFDataGetBytePtr(data);
			for (i = 0, j = 0; i < n; i += 2, j++) {
				unsigned long	byte;
				char		*end;
				char		str[3]	= { 0 };

				str[0] = argv[0][i];
				str[1] = argv[0][i + 1];
				errno = 0;
				byte = strtoul(str, &end, 16);
				if ((*end != '\0') || (errno != 0)) {
					CFRelease(data);
					data = NULL;
					break;
				}

				bytes[j] = byte;
			}

			if (data == NULL) {
				SCPrint(TRUE, stdout, CFSTR("d.add: invalid data.\n"));
				if (doArray) CFRelease(array);
				CFRelease(key);
				return;
			}

			val = data;
		} else if (doNumeric) {
			int	intValue;

			if (sscanf(argv[0], "%d", &intValue) == 1) {
				val = CFNumberCreate(NULL, kCFNumberIntType, &intValue);
			} else {
				SCPrint(TRUE, stdout, CFSTR("d.add: invalid data.\n"));
				if (doArray) CFRelease(array);
				CFRelease(key);
				return;
			}
		} else {
			val = (CFPropertyListRef)CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
		}

		if (doArray) {
			CFArrayAppendValue(array, val);
			CFRelease(val);
			argv++; argc--;
		} else {
			break;
		}
	}

	newValue = CFDictionaryCreateMutableCopy(NULL, 0, value);
	if (doArray) {
		CFDictionarySetValue(newValue, key, array);
		CFRelease(array);
	} else if (val != NULL) {
		CFDictionarySetValue(newValue, key, val);
		CFRelease(val);
	}
	CFRelease(key);
	CFRelease(value);
	value = newValue;

	return;
}


__private_extern__
void
do_dictRemoveKey(int argc, char **argv)
{
	CFStringRef		key;
	CFMutableDictionaryRef	val;

	if (value == NULL) {
		SCPrint(TRUE, stdout, CFSTR("d.remove: dictionary must be initialized.\n"));
		return;
	}

	if (!isA_CFDictionary(value)) {
		SCPrint(TRUE, stdout, CFSTR("d.remove: data (fetched from configuration server) is not a dictionary.\n"));
		return;
	}

	val = CFDictionaryCreateMutableCopy(NULL, 0, value);
	CFRelease(value);
	value = val;

	key = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	CFDictionaryRemoveValue((CFMutableDictionaryRef)value, key);
	CFRelease(key);

	return;
}
