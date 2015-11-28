/*
 * Copyright (c) 2000-2004, 2010, 2011 Apple Inc. All rights reserved.
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
#include "cache.h"
#include "session.h"
#include "notifications.h"


static void
reconnected(SCDynamicStoreRef store, void *info)
{
	SCPrint(TRUE, stdout, CFSTR("SCDynamicStore server restarted, session reconnected\n"));
	return;
}


__private_extern__
void
do_open(int argc, char **argv)
{
	if (store) {
		CFRelease(store);
		CFRelease(watchedKeys);
		CFRelease(watchedPatterns);
	}

	if (argc < 1) {
		store = SCDynamicStoreCreate(NULL, CFSTR("scutil"), storeCallback, NULL);
	} else {
		CFMutableDictionaryRef	options;

		options = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(options, kSCDynamicStoreUseSessionKeys, kCFBooleanTrue);
		store = SCDynamicStoreCreateWithOptions(NULL,
							CFSTR("scutil"),
							options,
							storeCallback,
							NULL);
		CFRelease(options);
	}
	if (store == NULL) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		return;
	}

	(void) SCDynamicStoreSetDisconnectCallBack(store, reconnected);

	watchedKeys     = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	watchedPatterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	cache_close();

	return;
}


__private_extern__
void
do_close(int argc, char **argv)
{
	if (notifyRls != NULL) {
		if (doDispatch) {
			(void) SCDynamicStoreSetDispatchQueue(store, NULL);
		} else {
			CFRunLoopSourceInvalidate(notifyRls);
			CFRelease(notifyRls);
		}
		notifyRls = NULL;
	}

	if (notifyRl != NULL) {
		CFRunLoopStop(notifyRl);
		notifyRl  = NULL;
	}

	if (store != NULL) {
		CFRelease(store);
		store = NULL;
		CFRelease(watchedKeys);
		watchedKeys = NULL;
		CFRelease(watchedPatterns);
		watchedPatterns = NULL;
	}

	cache_close();

	return;
}
