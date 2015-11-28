/*
 * Copyright (c) 2012-2014 Apple Inc. All rights reserved.
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

#include <Availability.h>
#include <TargetConditionals.h>
#include <asl.h>
#include <dispatch/dispatch.h>
#include <vproc.h>
#include <vproc_priv.h>
#include <xpc/xpc.h>
#include <xpc/private.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCPrivate.h>

#include "libSystemConfiguration_server.h"

#define kTrailingEdgeAgentEntitlement "com.apple.SystemConfiguration.trailing-edge-agent"

#pragma mark -
#pragma mark Support functions


//__private_extern__ void
//log_xpc_object(const char *msg, xpc_object_t obj)
//{
//	char	*desc;
//
//	desc = xpc_copy_description(obj);
//	asl_log(NULL, NULL, ASL_LEVEL_ERR, "%s = %s", msg, desc);
//	free(desc);
//}


#pragma mark -
#pragma mark client connection trackng


typedef struct {
	xpc_connection_t	connection;
} client_key_t;

typedef struct {
	pid_t			pid;
	uint64_t		generation_pushed;
	uint64_t		generation_acknowledged;
} client_val_t;


static __inline__ CF_RETURNS_RETAINED CFDataRef
_client_key(xpc_connection_t c)
{
	client_key_t	key;
	CFDataRef	client_key;

	key.connection = c;
	client_key = CFDataCreate(NULL, (UInt8 *)&key, sizeof(key));
	return client_key;
}


static void
_handle_entitlement_check_failure(pid_t pid)
{
	static Boolean			cleanupScheduled	= FALSE;
	static dispatch_once_t		initializer		= 0;
	static CFMutableArrayRef	pids			= NULL;
	static dispatch_queue_t		queue			= NULL;

	dispatch_once(&initializer, ^{
		pids = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		queue = dispatch_queue_create("handle unentitled ack", NULL);
	});

	dispatch_sync(queue, ^{
		CFNumberRef	pidNumber	= CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pid);

		if (!CFArrayContainsValue(pids, CFRangeMake(0, CFArrayGetCount(pids)), pidNumber)) {
			CFArrayAppendValue(pids, pidNumber);

			SCLog(TRUE, LOG_ERR, CFSTR("DNS/nwi dropping ack w/no entitlement, pid = %d"), pid);

			if (!cleanupScheduled) {
				cleanupScheduled = TRUE;
				dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 180LL * NSEC_PER_SEC), queue, ^{
					CFArrayRemoveAllValues(pids);
					cleanupScheduled = FALSE;
				});
			}
		}

		CFRelease(pidNumber);
	});
}


/*
 * libSystemConfiguraiton_client
 *
 * - all APIs must be called from the same [serial] dispatch queue
 */


__private_extern__
void
_libSC_info_server_init(libSC_info_server_t *server_info) {
	bzero(server_info, sizeof(*server_info));
	server_info->info = CFDictionaryCreateMutable(NULL,
						      0,
						      &kCFTypeDictionaryKeyCallBacks,
						      &kCFTypeDictionaryValueCallBacks);
	return;
}


__private_extern__
void
_libSC_info_server_set_data(libSC_info_server_t	*server_info,
			    CFDataRef		data,
			    uint64_t		generation)
{
	// update stored configuration
	if (server_info->data != NULL) {
		CFRelease(server_info->data);
		server_info->data = NULL;
	}
	if (data != NULL) {
		CFRetain(data);
		server_info->data = data;
	}

	// update generation
	if (generation == 0) {
		// generation must be non-zero
		generation = 1;
	}
	server_info->generation = generation;

	// new configuration, all ack'ing clients need to
	// check-in again
	server_info->inSync_NO += server_info->inSync_YES;
	server_info->inSync_YES = 0;

	return;
}


/*
 * _libSC_info_server_in_sync
 *
 * Called to check if all of the "active" configuration [XPC] connection
 * are in sync with the requested generation.
 */
__private_extern__
Boolean
_libSC_info_server_in_sync(libSC_info_server_t *server_info)
{
	return (server_info->inSync_NO == 0) ? TRUE : FALSE;
}


/*
 * _libSC_info_server_open
 *
 * Called when a new configuration [XPC] connection
 * is established.
 *
 * - tracks the last generation pushed to the caller and
 *   the last generation ack'd by the caller
 */
__private_extern__
void
_libSC_info_server_open(libSC_info_server_t	*server_info,
			xpc_connection_t	c)
{
	CFDataRef		client_key;
	CFMutableDataRef	client_val;
	client_val_t		*val;

	client_key = _client_key(c);

	client_val = CFDataCreateMutable(NULL, sizeof(*val));
	CFDataSetLength(client_val, sizeof(*val));

	val = (client_val_t *)(void *)CFDataGetMutableBytePtr(client_val);
	val->pid                     = xpc_connection_get_pid(c);
	val->generation_pushed       = 0;
	val->generation_acknowledged = 0;

	CFDictionarySetValue(server_info->info, client_key, client_val);
	CFRelease(client_key);
	CFRelease(client_val);

	return;
}


/*
 * _libSC_info_server_get_data
 *
 * Called when a [XPC] connection wants the current configuration.
 *
 * - updates the last generation pushed to the caller
 */
__private_extern__
CFDataRef
_libSC_info_server_get_data(libSC_info_server_t	*server_info,
			    xpc_connection_t	c,
			    uint64_t		*generation)
{
	CFDataRef		client_key;
	CFMutableDataRef	client_val;
	client_val_t		*val;

	// update last generation pushed to client
	client_key = _client_key(c);
	client_val = (CFMutableDataRef)CFDictionaryGetValue(server_info->info, client_key);
	CFRelease(client_key);

	val = (client_val_t *)(void *)CFDataGetMutableBytePtr(client_val);
	val->generation_pushed = server_info->generation;

	// return generation
	*generation = server_info->generation;
	if (*generation == 1) {
		*generation = 0;
	}

	// return data
	return server_info->data;
}


/*
 * _libSC_info_server_acknowledged
 *
 * Called when a [XPC] connection wants to acknowledge a
 * processed configuration.
 *
 * - updates the last generation ack'd by the caller
 * - updates the count of [XPC] connections that are / not in sync
 */
__private_extern__
Boolean
_libSC_info_server_acknowledged(libSC_info_server_t	*server_info,
				xpc_connection_t	c,
				uint64_t		generation)
{
	CFDataRef		client_key;
	CFMutableDataRef	client_val;
	xpc_object_t		ent_value;
	Boolean			entitled	= FALSE;
	Boolean			sync_updated	= FALSE;
	client_val_t		*val;

	ent_value = xpc_connection_copy_entitlement_value(c, kTrailingEdgeAgentEntitlement);
	if (ent_value != NULL) {
		if (xpc_get_type(ent_value) == XPC_TYPE_BOOL) {
			entitled = xpc_bool_get_value(ent_value);
		}
		xpc_release(ent_value);
	}

	if (!entitled) {
		_handle_entitlement_check_failure(xpc_connection_get_pid(c));
		return FALSE;
	}

	client_key = _client_key(c);
	client_val = (CFMutableDataRef)CFDictionaryGetValue(server_info->info, client_key);
	CFRelease(client_key);

	val = (client_val_t *)(void *)CFDataGetMutableBytePtr(client_val);

	if (val->generation_acknowledged == 0) {
		// if first ack
		if (generation == server_info->generation) {
			server_info->inSync_YES++;
		} else {
			server_info->inSync_NO++;
			sync_updated = TRUE;
		}
	} else if ((generation != val->generation_acknowledged) &&
		    (generation == server_info->generation)) {
		// if we've previously ack'd a configuration
		// ... and if we are ack'ing a configuration
		//     that we have not previously ack'd
		// ... and if we're ack'ing the current stored
		//     configuration
		server_info->inSync_NO--;
		server_info->inSync_YES++;
		sync_updated = TRUE;
	}

	val->generation_acknowledged = generation;

	return sync_updated;
}


/*
 * _libSC_info_server_close
 *
 * Called when a configuration [XPC] connection is closed.
 */
__private_extern__
Boolean
_libSC_info_server_close(libSC_info_server_t	*server_info,
			 xpc_connection_t	c)
{
	CFDataRef		client_key;
	CFMutableDataRef	client_val;
	Boolean			sync_updated	= FALSE;

	client_key = _client_key(c);

	// get client info, remove ack'd info
	client_val = (CFMutableDataRef)CFDictionaryGetValue(server_info->info, client_key);
	if (client_val != NULL) {
		client_val_t	*val;

		val = (client_val_t *)(void *)CFDataGetMutableBytePtr(client_val);
		if (val->generation_acknowledged > 0) {
			// if we've previously ack'd a configuration
			if (val->generation_acknowledged == server_info->generation) {
				// if currently in sync
				server_info->inSync_YES--;
			} else {
				// if currently NOT in sync
				server_info->inSync_NO--;
				sync_updated = TRUE;
			}
		}
	}

	CFDictionaryRemoveValue(server_info->info, client_key);
	CFRelease(client_key);

	return sync_updated;
}

