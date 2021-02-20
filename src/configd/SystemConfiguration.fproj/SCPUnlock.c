/*
 * Copyright (c) 2000, 2001, 2004-2010, 2013, 2015, 2016, 2018 Apple Inc. All rights reserved.
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

	__SCPreferencesUpdateLockedState(prefs, FALSE);
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
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;

	SC_log(LOG_ERR,
	       "SCPreferences(%@:%@) lock held for %d.%3.3d seconds",
	       prefsPrivate->name,
	       prefsPrivate->prefsID,
	       (int)delay->tv_sec,
	       delay->tv_usec / 1000);
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

	if (prefsPrivate->sessionNoO_EXLOCK != NULL) {
		// Note: closing the session removes the temporary "lock" key
		CFRelease(prefsPrivate->sessionNoO_EXLOCK);
		prefsPrivate->sessionNoO_EXLOCK = NULL;
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

	SC_log(LOG_DEBUG, "SCPreferences() unlock: %s",
	       prefsPrivate->newPath ? prefsPrivate->newPath : prefsPrivate->path);

	__SCPreferencesUpdateLockedState(prefs, FALSE);

	pthread_mutex_unlock(&prefsPrivate->lock);
	return TRUE;
}
