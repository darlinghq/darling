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

/*
 * Modification History
 *
 * February 8, 2012		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <notify.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>
#include <CommonCrypto/CommonDigest.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCPrivate.h>

#include "libSystemConfiguration_client.h"
#include "libSystemConfiguration_server.h"

#include <network_information.h>
#include "network_information_priv.h"
#include "network_information_server.h"


#pragma mark -
#pragma mark Globals


/*
 * S_nwi_info
 *
 * Note: all accesses should be made while running on the _nwi_server_queue()
 */
static libSC_info_server_t	S_nwi_info;


/*
 * S_debug
 *   A boolean that enables additional logging.
 */
static Boolean		*S_debug	= NULL;
static SCLoggerRef	S_logger	= NULL;


/*
 * S_sync_handler
 *	ACK (in-sync or not-in-sync) updates should be posted using
 *	this handler
 *
 * Note: all accesses should be made while running on the _nwi_server_queue()
 */
static _nwi_sync_handler_t	S_sync_handler	= NULL;


#pragma mark -
#pragma mark Support functions


#ifdef	NOT_YET_NEEDED
static void
log_xpc_object(const char *msg, xpc_object_t obj)
{
	char		*desc;

	desc = xpc_copy_description(obj);
	if (*S_debug) {
		SCLoggerLog(S_logger, LOG_DEBUG, "%s = %s", msg, desc);
	}
	free(desc);
}
#endif


#pragma mark -
#pragma mark Network information server "main"


static dispatch_queue_t
_nwi_state_server_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(NWI_SERVICE_NAME ".server", NULL);
	});

	return q;
}


/*
 * _nwi_state_copy
 *
 * Called when a client wants a copy of the current
 * Network information
 *
 * - caller must be running on the _nwi_server_queue()
 */
static void
_nwi_state_copy(xpc_connection_t connection, xpc_object_t request)
{
	CFDataRef		data;
	uint64_t		generation;
	xpc_connection_t	remote;
	xpc_object_t		reply;

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);
	if (reply == NULL) {
		SCLoggerLog(S_logger, LOG_ERR,
			    CFSTR("<%p> _nwi_state_copy: xpc_dictionary_create_reply: failed"),
			    connection);
		return;
	}

	// extract data and generation #
	data = _libSC_info_server_get_data(&S_nwi_info, connection, &generation);

	if (*S_debug) {
		const char	*proc_name;

		// extract process name
		proc_name = xpc_dictionary_get_string(request, NWI_PROC_NAME);
		if (proc_name == NULL) {
			proc_name = "???";
		}

		SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("<%p:%s[%d]> Network information copy: %llu"),
			    connection,
			    proc_name,
			    xpc_connection_get_pid(connection),
			    generation);
	}

	// return the Network information (if available)
	if (data != NULL) {
		xpc_dictionary_set_data(reply,
					NWI_CONFIGURATION,
					CFDataGetBytePtr(data),
					CFDataGetLength(data));
	}

	// reply
	xpc_connection_send_message(remote, reply);
	xpc_release(reply);

	return;
}


/*
 * _nwi_state_acknowledge
 *
 * Called when a client wants to acknowledge processing
 * of the Network information
 *
 * - caller must be running on the _nwi_server_queue()
 */
static void
_nwi_state_acknowledge(xpc_connection_t connection, xpc_object_t request)
{
	Boolean		changed;
	uint64_t	generation;

	generation = xpc_dictionary_get_uint64(request, NWI_GENERATION);

	if (*S_debug) {
		SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("<%p:%d> Network information ack: %llu"),
			    connection,
			    xpc_connection_get_pid(connection),
			    generation);
	}

	_libSC_info_server_acknowledged(&S_nwi_info, connection, generation);
	changed = _libSC_info_server_acknowledged(&S_nwi_info, connection, generation);
	if (changed) {
		Boolean		inSync;

		// report change
		inSync = _libSC_info_server_in_sync(&S_nwi_info);
		S_sync_handler(inSync);
	}

	return;
}


static void
process_request(xpc_connection_t connection, xpc_object_t request)
{
	int64_t		op;

	op = xpc_dictionary_get_int64(request, NWI_REQUEST);
	switch (op) {
		case NWI_REQUEST_COPY :
			/*
			 * Return the Network information
			 */
			_nwi_state_copy(connection, request);
			break;

		case NWI_REQUEST_ACKNOWLEDGE :
			/*
			 * Acknowlege a [processed] Network information
			 */
			_nwi_state_acknowledge(connection, request);

			break;
		default :
			SCLoggerLog(S_logger, LOG_ERR,
				    CFSTR("<%p> unknown request : %lld"),
				    connection,
				    op);

			break;
	}

	return;
}


static void
process_new_connection(xpc_connection_t c)
{
	if (*S_debug) {
		SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("<%p:%d> Network information session: open"),
			    c,
			    xpc_connection_get_pid(c));
	}

	_libSC_info_server_open(&S_nwi_info, c);

	xpc_connection_set_target_queue(c, _nwi_state_server_queue());

	xpc_connection_set_event_handler(c, ^(xpc_object_t xobj) {
		xpc_type_t	type;

		type = xpc_get_type(xobj);
		if (type == XPC_TYPE_DICTIONARY) {
			// process the request
			process_request(c, xobj);

		} else if (type == XPC_TYPE_ERROR) {
			const char	*desc;

			desc = xpc_dictionary_get_string(xobj, XPC_ERROR_KEY_DESCRIPTION);
			if (xobj == XPC_ERROR_CONNECTION_INVALID) {
				Boolean		changed;

				if (*S_debug) {
					SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("<%p:%d> Network information session: close"),
						    c,
						    xpc_connection_get_pid(c));
				}

				changed = _libSC_info_server_close(&S_nwi_info, c);
				if (changed) {
					Boolean		inSync;

					// report change
					inSync = _libSC_info_server_in_sync(&S_nwi_info);
					S_sync_handler(inSync);
				}

			} else if (xobj == XPC_ERROR_CONNECTION_INTERRUPTED) {
				SCLoggerLog(S_logger, LOG_ERR,
					    CFSTR("<%p:%d> %s"),
					    c,
					    xpc_connection_get_pid(c),
					    desc);

			} else {
				SCLoggerLog(S_logger, LOG_ERR,
					    CFSTR("<%p:%d> Connection error: %p : %s"),
					    c,
					    xpc_connection_get_pid(c),
					    xobj,
					    desc);
			}

		}  else {
			SCLoggerLog(S_logger, LOG_ERR,
				    CFSTR("<%p:%d> unknown event type : %p"),
				    c,
				    xpc_connection_get_pid(c),
				    type);
		}
	});

	xpc_connection_resume(c);

	return;
}


#pragma mark -
#pragma mark Network Information server SPIs


__private_extern__
void
load_NetworkInformation(CFBundleRef		bundle,
			SCLoggerRef		logger,
			Boolean			*bundleVerbose,
			_nwi_sync_handler_t	syncHandler)
{
	xpc_connection_t	c;
	const char		*name;

	S_debug = bundleVerbose;
	S_logger = logger;

	/*
	 * keep track of Network information acknowledgements
	 */
	_libSC_info_server_init(&S_nwi_info);

	/*
	 * save the in-sync/not-in-sync handler
	 */
	S_sync_handler = Block_copy(syncHandler);

	// create XPC listener
	name = getenv(NWI_SERVICE_NAME);
	if (name == NULL) {
		name = NWI_SERVICE_NAME;
	}

	c = xpc_connection_create_mach_service(name,
					       _nwi_state_server_queue(),
					       XPC_CONNECTION_MACH_SERVICE_LISTENER);

	xpc_connection_set_event_handler(c, ^(xpc_object_t event) {
		xpc_type_t	type;

		type = xpc_get_type(event);
		if (type == XPC_TYPE_CONNECTION) {
			process_new_connection(event);

		} else if (type == XPC_TYPE_ERROR) {
			const char	*desc;

			desc = xpc_dictionary_get_string(event, XPC_ERROR_KEY_DESCRIPTION);
			if (event == XPC_ERROR_CONNECTION_INVALID) {
				SCLoggerLog(S_logger, LOG_ERR, CFSTR("Network information server: %s"), desc);
				xpc_release(c);
			} else if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
				SCLoggerLog(S_logger, LOG_ERR, CFSTR("Network information server: %s"), desc);
			} else {
				SCLoggerLog(S_logger, LOG_ERR,
					    CFSTR("Network information server: Connection error: %p : %s"),
					    event,
					    desc);
			}

		} else {
			SCLoggerLog(S_logger, LOG_ERR,
				    CFSTR("Network information server: unknown event type : %p"),
				    type);

		}
	});

	xpc_connection_resume(c);

SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("XPC server \"%s\" started"), name);

	return;
}


__private_extern__
void
_nwi_state_signature(nwi_state		*state,
		     unsigned char	*signature,
		     size_t		signature_len)
{
	bzero(signature, signature_len);

	if (state != NULL) {
		CC_SHA1_CTX	ctx;
		uint64_t	generation_save;
		unsigned char	*sha1;
		unsigned char	sha1_buf[CC_SHA1_DIGEST_LENGTH];

		generation_save = state->generation_count;
		state->generation_count = 0;

		sha1 = (signature_len >= CC_SHA1_DIGEST_LENGTH) ? signature : sha1_buf;
		CC_SHA1_Init(&ctx);
		CC_SHA1_Update(&ctx,
			       state,
			       state->size);
		CC_SHA1_Final(sha1, &ctx);
		if (sha1 != signature) {
			bcopy(sha1, signature, signature_len);
		}

		state->generation_count = generation_save;
	}

	return;
}


__private_extern__
_Bool
_nwi_state_store(nwi_state *state)
{
	Boolean		in_sync;
	uint64_t	new_generation	= 0;
	CFDataRef	new_nwi_info	= NULL;
	const char	*notify_key;

	if (state != NULL) {
		const UInt8	*bytes;
		CFIndex		len;

		new_generation = state->generation_count;

		if (*S_debug) {
			SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("Network information updated: %llu"),
				    new_generation);
		}

		bytes = (const UInt8 *)state;
		len = state->size;

		new_nwi_info = CFDataCreate(NULL, bytes, len);
	}

	dispatch_sync(_nwi_state_server_queue(), ^{
		_libSC_info_server_set_data(&S_nwi_info, new_nwi_info, new_generation);
	});

	if (new_nwi_info != NULL) {
		CFRelease(new_nwi_info);
	}

	// if anyone is keeping us in sync, they now need to catchup
	in_sync = _libSC_info_server_in_sync(&S_nwi_info);
	S_sync_handler(in_sync);

	// and let everyone else know that the configuration has been updated
	notify_key = nwi_state_get_notify_key();
	if (notify_key != NULL) {
		uint32_t	status;

		_nwi_state_force_refresh();
		status = notify_post(notify_key);
		if (status != NOTIFY_STATUS_OK) {
			SCLoggerLog(S_logger, LOG_ERR, CFSTR("notify_post() failed: %d"), status);
			// notification posting failures are non-fatal
		}
	}

	return TRUE;
}


#pragma mark -
#pragma mark Testing


#ifdef  MAIN

int
main(int argc, char **argv)
{
	static Boolean verbose = (argc > 1) ? TRUE : FALSE;
	//	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;
	_sc_debug   = TRUE;

	load_NetworkInformation(CFBundleGetMainBundle(),	// bundle
				NULL,				// SCLogger
				&verbose,			// bundleVerbose
				^(Boolean inSync) {		// sync handler
				      SCLoggerLog(NULL, LOG_INFO,
					    CFSTR("in sync: %s"),
					    inSync ? "Yes" : "No");
				});
	CFRunLoopRun();
	/* not reached */
	exit(0);
	return 0;
}

#endif  /* MAIN */
