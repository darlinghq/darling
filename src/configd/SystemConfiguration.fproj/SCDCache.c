/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
 * Oct 1, 2018	Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#ifdef	SC_LOG_HANDLE
#include <os/log.h>
os_log_t	SC_LOG_HANDLE(void);
#endif	//SC_LOG_HANDLE

#include "SCDynamicStoreInternal.h"


Boolean
_SCDynamicStoreCacheIsActive(SCDynamicStoreRef store)
{
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (store == NULL) {
		// sorry, you must provide a session
		_SCErrorSet(kSCStatusNoStoreSession);
		return FALSE;
	}

	return storePrivate->cache_active;
}


static void
__SCDynamicStoreCacheRelease(SCDynamicStoreRef store)
{
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (storePrivate->cached_keys != NULL) {
		CFRelease(storePrivate->cached_keys);
		storePrivate->cached_keys = NULL;
	}
	if (storePrivate->cached_set != NULL) {
		CFRelease(storePrivate->cached_set);
		storePrivate->cached_set = NULL;
	}
	if (storePrivate->cached_removals != NULL) {
		CFRelease(storePrivate->cached_removals);
		storePrivate->cached_removals = NULL;
	}
	if (storePrivate->cached_notifys != NULL) {
		CFRelease(storePrivate->cached_notifys);
		storePrivate->cached_notifys = NULL;
	}

	return;
}


Boolean
_SCDynamicStoreCacheOpen(SCDynamicStoreRef store)
{
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (store == NULL) {
		// sorry, you must provide a session
		_SCErrorSet(kSCStatusNoStoreSession);
		return FALSE;
	}

	__SCDynamicStoreCacheRelease(store);	// if we are already using the cache, start clean
	storePrivate->cache_active = TRUE;

	return TRUE;
}


Boolean
_SCDynamicStoreCacheCommitChanges(SCDynamicStoreRef store)
{
	Boolean				ok		= TRUE;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (store == NULL) {
		// sorry, you must provide a session
		_SCErrorSet(kSCStatusNoStoreSession);
		return FALSE;
	}

	if (!storePrivate->cache_active) {
		// if not using the cache
		_SCErrorSet(kSCStatusFailed);
		return FALSE;
	}

	if ((storePrivate->cached_set != NULL) ||
	    (storePrivate->cached_removals != NULL) ||
	    (storePrivate->cached_notifys != NULL)) {
		ok = SCDynamicStoreSetMultiple(store,
					       storePrivate->cached_set,
					       storePrivate->cached_removals,
					       storePrivate->cached_notifys);
		__SCDynamicStoreCacheRelease(store);
	}

	return ok;
}


Boolean
_SCDynamicStoreCacheClose(SCDynamicStoreRef store)
{
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (store == NULL) {
		// sorry, you must provide a session
		_SCErrorSet(kSCStatusNoStoreSession);
		return FALSE;
	}

	if (!storePrivate->cache_active) {
		// if not using the cache
		_SCErrorSet(kSCStatusFailed);
		return FALSE;
	}

	__SCDynamicStoreCacheRelease(store);
	storePrivate->cache_active = FALSE;

	return TRUE;
}
