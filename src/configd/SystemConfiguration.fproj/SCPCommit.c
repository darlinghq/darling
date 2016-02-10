/*
 * Copyright (c) 2000-2008, 2010-2013 Apple Inc. All rights reserved.
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

#include <TargetConditionals.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCPreferencesInternal.h"
#include "SCHelper_client.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/errno.h>

static Boolean
__SCPreferencesCommitChanges_helper(SCPreferencesRef prefs)
{
	CFDataRef		data		= NULL;
	Boolean			ok;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	uint32_t		status		= kSCStatusOK;
	CFDataRef		reply		= NULL;

	if (prefsPrivate->helper_port == MACH_PORT_NULL) {
		// if no helper
		status = kSCStatusAccessError;
		goto fail;
	}

	if (prefsPrivate->changed) {
		ok = _SCSerialize(prefsPrivate->prefs, &data, NULL, NULL);
		if (!ok) {
			status = kSCStatusFailed;
			if (_sc_verbose) {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("SCPreferencesCommitChanges(-->helper) CFPropertyListCreateData() failed"));
				SCLog(TRUE, LOG_ERR,
				      CFSTR("  prefs = %s"),
				      prefsPrivate->newPath ? prefsPrivate->newPath : prefsPrivate->path);
			}
			goto error;
		}
	}

	// have the helper "commit" the prefs
//	status = kSCStatusOK;
//	reply  = NULL;
	ok = _SCHelperExec(prefsPrivate->helper_port,
			   SCHELPER_MSG_PREFS_COMMIT,
			   data,
			   &status,
			   &reply);
	if (data != NULL) CFRelease(data);
	if (!ok) {
		goto fail;
	}

	if (status != kSCStatusOK) {
		goto error;
	}

	if (prefsPrivate->changed) {
		if (prefsPrivate->signature != NULL) CFRelease(prefsPrivate->signature);
		prefsPrivate->signature = reply;
	} else {
		if (reply != NULL) CFRelease(reply);
	}

	prefsPrivate->changed = FALSE;
	return TRUE;

    fail :

	// close helper
	if (prefsPrivate->helper_port != MACH_PORT_NULL) {
		_SCHelperClose(&prefsPrivate->helper_port);
	}

    error :

	// return error
	if (reply != NULL) CFRelease(reply);
	_SCErrorSet(status);
	return FALSE;
}


static ssize_t
writen(int ref, const void *data, size_t len)
{
	size_t		left	= len;
	ssize_t		n;
	const void	*p	= data;

	while (left > 0) {
		if ((n = write(ref, p, left)) == -1) {
			if (errno != EINTR) {
				return -1;
			}
			n = 0;
		}
		left -= n;
		p += n;
	}
	return len;
}


Boolean
SCPreferencesCommitChanges(SCPreferencesRef prefs)
{
	Boolean			ok		= FALSE;
	char *			path;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	Boolean			save		= TRUE;
	struct stat		statBuf;
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
			SCLog(_sc_verbose, LOG_ERR, CFSTR("SCPreferencesCommitChanges SCPreferencesLock() failed"));
			return FALSE;
		}
	}

	if (prefsPrivate->authorizationData != NULL) {
		ok = __SCPreferencesCommitChanges_helper(prefs);
		if (ok) {
			prefsPrivate->changed = FALSE;
		}
		goto done;
	}

	/*
	 * if necessary, apply changes
	 */
	if (!prefsPrivate->changed) {
		goto committed;
	}

	/*
	 * check if the preferences should be removed
	 */
	if (CFDictionaryGetCount(prefsPrivate->prefs) == 0) {
		CFBooleanRef	val;

		/* if empty */
		if ((prefsPrivate->options != NULL) &&
		    CFDictionaryGetValueIfPresent(prefsPrivate->options,
						  kSCPreferencesOptionRemoveWhenEmpty,
						  (const void **)&val) &&
		    isA_CFBoolean(val) &&
		    CFBooleanGetValue(val)) {
			/* if we've been asked to remove empty .plists */
			save = FALSE;
		}
	}

	path = prefsPrivate->newPath ? prefsPrivate->newPath : prefsPrivate->path;
	if (save) {
		int		fd;
		CFDataRef	newPrefs;
		CFIndex		pathLen;
		char *		thePath;

		if (stat(prefsPrivate->path, &statBuf) == -1) {
			if (errno == ENOENT) {
				bzero(&statBuf, sizeof(statBuf));
				statBuf.st_mode = 0644;
				statBuf.st_uid  = geteuid();
				statBuf.st_gid  = getegid();
			} else {
				SCLog(_sc_verbose, LOG_ERR, CFSTR("SCPreferencesCommitChanges stat() failed: %s"), strerror(errno));
				goto done;
			}
		}

		/* create the (new) preferences file */
		pathLen = strlen(path) + sizeof("-new");
		thePath = CFAllocatorAllocate(NULL, pathLen, 0);
		snprintf(thePath, pathLen, "%s-new", path);

		fd = open(thePath, O_WRONLY|O_CREAT, statBuf.st_mode);
		if (fd == -1) {
			_SCErrorSet(errno);
			SCLog(_sc_verbose, LOG_ERR, CFSTR("SCPreferencesCommitChanges open() failed: %s"), strerror(errno));
			CFAllocatorDeallocate(NULL, thePath);
			goto done;
		}

		/* preserve permissions */
		(void) fchown(fd, statBuf.st_uid, statBuf.st_gid);
		(void) fchmod(fd, statBuf.st_mode);

		/* write the new preferences */
		newPrefs = CFPropertyListCreateData(NULL,
						    prefsPrivate->prefs,
#if	TARGET_OS_IPHONE
						    kCFPropertyListBinaryFormat_v1_0,
#else	// TARGET_OS_IPHONE
						    kCFPropertyListXMLFormat_v1_0,
#endif	// TARGET_OS_IPHONE
						    0,
						    NULL);
		if (!newPrefs) {
			_SCErrorSet(kSCStatusFailed);
			SCLog(_sc_verbose, LOG_ERR, CFSTR("SCPreferencesCommitChanges CFPropertyListCreateData() failed"));
			SCLog(_sc_verbose, LOG_ERR, CFSTR("  prefs = %s"), path);
			CFAllocatorDeallocate(NULL, thePath);
			(void) close(fd);
			goto done;
		}
		if (writen(fd, (const void *)CFDataGetBytePtr(newPrefs), CFDataGetLength(newPrefs)) == -1) {
			_SCErrorSet(errno);
			SCLog(_sc_verbose, LOG_ERR, CFSTR("SCPreferencesCommitChanges write() failed: %s"), strerror(errno));
			SCLog(_sc_verbose, LOG_ERR, CFSTR("  path = %s"), thePath);
			(void) unlink(thePath);
			CFAllocatorDeallocate(NULL, thePath);
			(void) close(fd);
			CFRelease(newPrefs);
			goto done;
		}

		/* new preferences have been written */
		if (close(fd) == -1) {
			_SCErrorSet(errno);
			SCLog(_sc_verbose, LOG_ERR, CFSTR("SCPreferencesCommitChanges close() failed: %s"), strerror(errno));
			SCLog(_sc_verbose, LOG_ERR, CFSTR("  path = %s"), thePath);
			(void) unlink(thePath);
			CFAllocatorDeallocate(NULL, thePath);
			CFRelease(newPrefs);
			goto done;
		}
		CFRelease(newPrefs);

		/* rename new->old */
		if (rename(thePath, path) == -1) {
			_SCErrorSet(errno);
			SCLog(_sc_verbose, LOG_ERR, CFSTR("SCPreferencesCommitChanges rename() failed: %s"), strerror(errno));
			SCLog(_sc_verbose, LOG_ERR, CFSTR("  path = %s --> %s"), thePath, path);
			CFAllocatorDeallocate(NULL, thePath);
			goto done;
		}
		CFAllocatorDeallocate(NULL, thePath);

		if (prefsPrivate->newPath) {
			/* prefs file saved in "new" directory */
			(void) unlink(prefsPrivate->path);
			(void) symlink(prefsPrivate->newPath, prefsPrivate->path);
			CFAllocatorDeallocate(NULL, prefsPrivate->path);
			prefsPrivate->path = path;
			prefsPrivate->newPath = NULL;
		}

		/* grab the new signature */
		if (stat(path, &statBuf) == -1) {
			_SCErrorSet(errno);
			SCLog(_sc_verbose, LOG_ERR, CFSTR("SCPreferencesCommitChanges stat() failed: %s"), strerror(errno));
			SCLog(_sc_verbose, LOG_ERR, CFSTR("  path = %s"), thePath);
			goto done;
		}
	} else {
		/* remove the empty .plist */
		unlink(path);

		/* init the new signature */
		bzero(&statBuf, sizeof(statBuf));
	}

	/* update signature */
	if (prefsPrivate->signature != NULL) CFRelease(prefsPrivate->signature);
	prefsPrivate->signature = __SCPSignatureFromStatbuf(&statBuf);

    committed :

	/* post notification */
	if (prefsPrivate->session == NULL) {
		ok = TRUE;
	} else {
		ok = SCDynamicStoreNotifyValue(prefsPrivate->session, prefsPrivate->sessionKeyCommit);
		if (!ok) {
			SCLog(_sc_verbose, LOG_ERR, CFSTR("SCPreferencesCommitChanges SCDynamicStoreNotifyValue() failed"));
			_SCErrorSet(kSCStatusFailed);
			goto done;
		}
	}

	prefsPrivate->changed = FALSE;

    done :

	if (!wasLocked) {
		uint32_t	status;

		status = SCError();	// preserve status across unlock
		(void) SCPreferencesUnlock(prefs);
		_SCErrorSet(status);
	}
	return ok;
}
