/*
 * Copyright (c) 2000-2005, 2016 Apple Computer, Inc. All rights reserved.
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

#include "SCPreferencesInternal.h"

CFArrayRef
SCPreferencesCopyKeyList(SCPreferencesRef prefs)
{
	CFAllocatorRef		allocator;
	CFArrayRef		keys;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	CFIndex			prefsCnt;
	const void **		prefsKeys;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return NULL;
	}

	__SCPreferencesAccess(prefs);

	allocator = CFGetAllocator(prefs);
	prefsCnt  = CFDictionaryGetCount(prefsPrivate->prefs);
	if (prefsCnt > 0) {
		prefsKeys = CFAllocatorAllocate(allocator, prefsCnt * sizeof(CFStringRef), 0);
		CFDictionaryGetKeysAndValues(prefsPrivate->prefs, prefsKeys, NULL);
		keys = CFArrayCreate(allocator, prefsKeys, prefsCnt, &kCFTypeArrayCallBacks);
		CFAllocatorDeallocate(allocator, prefsKeys);
	} else {
		keys = CFArrayCreate(allocator, NULL, 0, &kCFTypeArrayCallBacks);
	}

	return keys;
}
