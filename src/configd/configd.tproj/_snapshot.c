/*
 * Copyright (c) 2000-2006, 2009-2011 Apple Inc. All rights reserved.
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
 * April 14, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <fcntl.h>
#include <paths.h>
#include <unistd.h>

#include "configd.h"
#include "configd_server.h"
#include "session.h"
#include "plugin_support.h"


#define	SNAPSHOT_PATH_STATE	_PATH_VARTMP "configd-state"
#define	SNAPSHOT_PATH_STORE	_PATH_VARTMP "configd-store.plist"
#define	SNAPSHOT_PATH_PATTERN	_PATH_VARTMP "configd-pattern.plist"
#define	SNAPSHOT_PATH_SESSION	_PATH_VARTMP "configd-session.plist"


#define N_QUICK	100

static CF_RETURNS_RETAINED CFDictionaryRef
_expandStore(CFDictionaryRef storeData)
{
	const void *		keys_q[N_QUICK];
	const void **		keys		= keys_q;
	CFIndex			nElements;
	CFDictionaryRef		newStoreData	= NULL;
	const void *		nValues_q[N_QUICK];
	const void **		nValues		= nValues_q;
	const void *		oValues_q[N_QUICK];
	const void **		oValues		= oValues_q;

	nElements = CFDictionaryGetCount(storeData);
	if (nElements > 0) {
		CFIndex	i;

		if (nElements > (CFIndex)(sizeof(keys_q) / sizeof(CFTypeRef))) {
			keys    = CFAllocatorAllocate(NULL, nElements * sizeof(CFTypeRef), 0);
			oValues = CFAllocatorAllocate(NULL, nElements * sizeof(CFTypeRef), 0);
			nValues = CFAllocatorAllocate(NULL, nElements * sizeof(CFTypeRef), 0);
		}
		bzero(nValues, nElements * sizeof(CFTypeRef));

		CFDictionaryGetKeysAndValues(storeData, keys, oValues);
		for (i = 0; i < nElements; i++) {
			CFDataRef		data;

			data = CFDictionaryGetValue(oValues[i], kSCDData);
			if (data) {
				CFPropertyListRef	plist;

				nValues[i] = CFDictionaryCreateMutableCopy(NULL, 0, oValues[i]);

				_SCUnserialize(&plist, data, NULL, 0);
				CFDictionarySetValue((CFMutableDictionaryRef)nValues[i],
						     kSCDData,
						     plist);
				CFRelease(plist);
			} else {
				nValues[i] = CFRetain(oValues[i]);
			}
		}
	}

	newStoreData = CFDictionaryCreate(NULL,
				     keys,
				     nValues,
				     nElements,
				     &kCFTypeDictionaryKeyCallBacks,
				     &kCFTypeDictionaryValueCallBacks);

	if (nElements > 0) {
		CFIndex	i;

		for (i = 0; i < nElements; i++) {
			CFRelease(nValues[i]);
		}

		if (keys != keys_q) {
			CFAllocatorDeallocate(NULL, keys);
			CFAllocatorDeallocate(NULL, oValues);
			CFAllocatorDeallocate(NULL, nValues);
		}
	}

	return newStoreData;
}


__private_extern__
int
__SCDynamicStoreSnapshot(SCDynamicStoreRef store)
{
	CFDictionaryRef			expandedStoreData;
	FILE				*f;
	int				fd;
	CFDataRef			xmlData;

	/* Save a snapshot of configd's "state" */

	(void) unlink(SNAPSHOT_PATH_STATE);
	fd = open(SNAPSHOT_PATH_STATE, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0644);
	if (fd == -1) {
		return kSCStatusFailed;
	}
	f = fdopen(fd, "w");
	if (f == NULL) {
		return kSCStatusFailed;
	}
	SCPrint(TRUE, f, CFSTR("Main thread :\n\n"));
	SCPrint(TRUE, f, CFSTR("%@\n"), CFRunLoopGetCurrent());
	if (plugin_runLoop != NULL) {
		SCPrint(TRUE, f, CFSTR("Plug-in thread :\n\n"));
		SCPrint(TRUE, f, CFSTR("%@\n"), plugin_runLoop);
	}
	listSessions(f);
	(void) fclose(f);

	/* Save a snapshot of the "store" data */

	(void) unlink(SNAPSHOT_PATH_STORE);
	fd = open(SNAPSHOT_PATH_STORE, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0644);
	if (fd == -1) {
		return kSCStatusFailed;
	}

	expandedStoreData = _expandStore(storeData);
	xmlData = CFPropertyListCreateData(NULL, expandedStoreData, kCFPropertyListXMLFormat_v1_0, 0, NULL);
	CFRelease(expandedStoreData);
	if (xmlData == NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("__SCDynamicStoreSnapshot CFPropertyListCreateData() failed"));
		close(fd);
		return kSCStatusFailed;
	}
	(void) write(fd, CFDataGetBytePtr(xmlData), CFDataGetLength(xmlData));
	(void) close(fd);
	CFRelease(xmlData);

	/* Save a snapshot of the "pattern" data */

	(void) unlink(SNAPSHOT_PATH_PATTERN);
	fd = open(SNAPSHOT_PATH_PATTERN, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0644);
	if (fd == -1) {
		return kSCStatusFailed;
	}

	xmlData = CFPropertyListCreateData(NULL, patternData, kCFPropertyListXMLFormat_v1_0, 0, NULL);
	if (xmlData == NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("__SCDynamicStoreSnapshot CFPropertyListCreateData() failed"));
		close(fd);
		return kSCStatusFailed;
	}
	(void) write(fd, CFDataGetBytePtr(xmlData), CFDataGetLength(xmlData));
	(void) close(fd);
	CFRelease(xmlData);

	/* Save a snapshot of the "session" data */

	(void) unlink(SNAPSHOT_PATH_SESSION);
	fd = open(SNAPSHOT_PATH_SESSION, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0644);
	if (fd == -1) {
		return kSCStatusFailed;
	}

	xmlData = CFPropertyListCreateData(NULL, sessionData, kCFPropertyListXMLFormat_v1_0, 0, NULL);
	if (xmlData == NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("__SCDynamicStoreSnapshot CFPropertyListCreateData() failed"));
		close(fd);
		return kSCStatusFailed;
	}
	(void) write(fd, CFDataGetBytePtr(xmlData), CFDataGetLength(xmlData));
	(void) close(fd);
	CFRelease(xmlData);

	return kSCStatusOK;
}


__private_extern__
kern_return_t
_snapshot(mach_port_t server, int *sc_status, audit_token_t audit_token)
{
	serverSessionRef	mySession;

	mySession = getSession(server);
	if (mySession == NULL) {
		mySession = tempSession(server, CFSTR("SCDynamicStoreSnapshot"), audit_token);
		if (mySession == NULL) {
			/* you must have an open session to play */
			return kSCStatusNoStoreSession;
		}
	}

	if (!hasRootAccess(mySession)) {
		return kSCStatusAccessError;
	}

	*sc_status = __SCDynamicStoreSnapshot(mySession->store);
	return KERN_SUCCESS;
}
