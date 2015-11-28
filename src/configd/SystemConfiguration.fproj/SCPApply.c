/*
 * Copyright (c) 2000, 2001, 2004-2006, 2010 Apple Inc. All rights reserved.
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


static Boolean
__SCPreferencesApplyChanges_helper(SCPreferencesRef prefs)
{
	Boolean			ok;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	uint32_t		status		= kSCStatusOK;

	if (prefsPrivate->helper_port == MACH_PORT_NULL) {
		// if no helper
		goto fail;
	}

	// have the helper "apply" the prefs
//	status = kSCStatusOK;
	ok = _SCHelperExec(prefsPrivate->helper_port,
			   SCHELPER_MSG_PREFS_APPLY,
			   NULL,
			   &status,
			   NULL);
	if (!ok) {
		goto fail;
	}

	if (status != kSCStatusOK) {
		goto error;
	}

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


Boolean
SCPreferencesApplyChanges(SCPreferencesRef prefs)
{
	Boolean			ok		= FALSE;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	Boolean			wasLocked;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return FALSE;
	}

	/*
	 * Determine if the we have exclusive access to the preferences
	 * and acquire the lock if necessary.
	 */
	wasLocked = prefsPrivate->locked;
	if (!wasLocked) {
		if (!SCPreferencesLock(prefs, TRUE)) {
			SCLog(_sc_verbose, LOG_DEBUG, CFSTR("SCPreferencesApplyChanges SCPreferencesLock() failed"));
			return FALSE;
		}
	}

	if (prefsPrivate->authorizationData != NULL) {
		ok = __SCPreferencesApplyChanges_helper(prefs);
		goto done;
	}

	/* post notification */
	if (prefsPrivate->session == NULL) {
		ok = TRUE;
	} else {
		ok = SCDynamicStoreNotifyValue(prefsPrivate->session, prefsPrivate->sessionKeyApply);
		if (!ok) {
			SCLog(_sc_verbose, LOG_DEBUG, CFSTR("SCPreferencesApplyChanges SCDynamicStoreNotifyValue() failed"));
			_SCErrorSet(kSCStatusFailed);
		}
	}

    done :

	if (!wasLocked) {
		uint32_t	status;

		status = SCError();	// preserve status across unlock
		(void) SCPreferencesUnlock(prefs);
		_SCErrorSet(status);
	}
	return ok;
}
