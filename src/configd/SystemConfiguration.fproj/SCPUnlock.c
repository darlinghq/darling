/*
 * Copyright (c) 2000, 2001, 2004-2010, 2013 Apple Inc. All rights reserved.
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

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCPreferencesInternal.h"
#include "SCHelper_client.h"

#include <unistd.h>
#include <pthread.h>

static Boolean
__SCPreferencesUnlock_helper(SCPreferencesRef prefs)
{
	Boolean			ok;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	uint32_t		status		= kSCStatusOK;

	if (prefsPrivate->helper_port == MACH_PORT_NULL) {
		// if no helper
		goto fail;
	}

	// have the helper "unlock" the prefs
//	status = kSCStatusOK;
	ok = _SCHelperExec(prefsPrivate->helper_port,
			   SCHELPER_MSG_PREFS_UNLOCK,
			   NULL,
			   &status,
			   NULL);
	if (!ok) {
		goto fail;
	}

	if (status != kSCStatusOK) {
		goto error;
	}

	prefsPrivate->locked = FALSE;
	return TRUE;

    fail :

	// close helper
	if (prefsPrivate->helper_port != MACH_PORT_NULL) {
		_SCHelperClose(&prefsPrivate->helper_port);
	}

	status = kSCStatusAccessError;

    error :

	// return error
	_SCErrorSet(status);
	return FALSE;
}


static void
reportDelay(SCPreferencesRef prefs, struct timeval *delay)
{
	asl_object_t		m;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	char			str[256];

	m = asl_new(ASL_TYPE_MSG);
	asl_set(m, "com.apple.message.domain", "com.apple.SystemConfiguration.SCPreferencesUnlock");
	(void) _SC_cfstring_to_cstring(prefsPrivate->name, str, sizeof(str), kCFStringEncodingUTF8);
	asl_set(m, "com.apple.message.signature", str);
	(void) _SC_cfstring_to_cstring(prefsPrivate->prefsID, str, sizeof(str), kCFStringEncodingUTF8);
	asl_set(m, "com.apple.message.signature2", str);
	(void) snprintf(str, sizeof(str),
			"%d.%3.3d",
			(int)delay->tv_sec,
			delay->tv_usec / 1000);
	asl_set(m, "com.apple.message.value", str);
	SCLOG(NULL, m, ASL_LEVEL_DEBUG,
	      CFSTR("SCPreferences(%@:%@) lock held for %d.%3.3d seconds"),
	      prefsPrivate->name,
	      prefsPrivate->prefsID,
	      (int)delay->tv_sec,
	      delay->tv_usec / 1000);
	asl_release(m);

	return;
}


Boolean
SCPreferencesUnlock(SCPreferencesRef prefs)
{
	struct timeval		lockElapsed;
	struct timeval		lockEnd;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return FALSE;
	}

	if (!prefsPrivate->locked) {
		/* sorry, you don't have the lock */
		_SCErrorSet(kSCStatusNeedLock);
		return FALSE;
	}

	if (prefsPrivate->authorizationData != NULL) {
		return __SCPreferencesUnlock_helper(prefs);
	}

	pthread_mutex_lock(&prefsPrivate->lock);

	if (prefsPrivate->sessionKeyLock != NULL) {
		SCDynamicStoreRemoveValue(prefsPrivate->session,
					  prefsPrivate->sessionKeyLock);
	}

	if (prefsPrivate->lockFD != -1)	{
		if (prefsPrivate->lockPath != NULL) {
			unlink(prefsPrivate->lockPath);
		}
		close(prefsPrivate->lockFD);
		prefsPrivate->lockFD = -1;
	}

	(void)gettimeofday(&lockEnd, NULL);
	timersub(&lockEnd, &prefsPrivate->lockTime, &lockElapsed);
	if (lockElapsed.tv_sec > 0) {
		// if we held the lock for more than 1 second
		reportDelay(prefs, &lockElapsed);
	}

	prefsPrivate->locked = FALSE;

	pthread_mutex_unlock(&prefsPrivate->lock);
	return TRUE;
}
