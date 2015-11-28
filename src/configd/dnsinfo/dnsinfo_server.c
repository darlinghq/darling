/*
 * Copyright (c) 2004-2008, 2011-2014 Apple Inc. All rights reserved.
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
 * March 9, 2004		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <notify.h>
#include <sysexits.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <servers/bootstrap.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <bsm/libbsm.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCPrivate.h>

#include "libSystemConfiguration_client.h"
#include "libSystemConfiguration_server.h"

#include "dnsinfo_create.h"
#include "dnsinfo_server.h"
#include "dnsinfo_private.h"

#pragma mark -
#pragma mark Globals


/*
 * S_dns_info
 *
 * Note: all accesses should be made while running on the _dns_server_queue()
 */
static libSC_info_server_t	S_dns_info;


/*
 * S_debug
 *	A boolean that enables additional logging.
 */
static Boolean		S_debug_s;
static Boolean *	S_debug = &S_debug_s;
static SCLoggerRef	S_logger = NULL;


/*
 * S_sync_handler
 *	ACK (in-sync or not-in-sync) updates should be posted using
 *	this handler
 *
 * Note: all accesses should be made while running on the _dns_server_queue()
 */
static _dns_sync_handler_t	S_sync_handler	= NULL;


#pragma mark -
#pragma mark Support functions


#ifdef	NOT_YET_NEEDED
static void
log_xpc_object(const char *msg, xpc_object_t obj)
{
	char		*desc;

	desc = xpc_copy_description(obj);
	if (*S_debug) {
		SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("%s = %s"), msg, desc);
	}
	free(desc);
}
#endif


#pragma mark -
#pragma mark DNS configuration server


static dispatch_queue_t
_dnsinfo_server_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(DNSINFO_SERVICE_NAME ".server", NULL);
	});

	return q;
}


/*
 * _dnsinfo_copy
 *
 * Called when a client wants a copy of the current
 * DNS configuration
 *
 * - caller must be running on the _dns_server_queue()
 */
static void
_dnsinfo_copy(xpc_connection_t connection, xpc_object_t request)
{
	CFDataRef		data;
	uint64_t		generation;
	xpc_connection_t	remote;
	xpc_object_t		reply;

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);
	if (reply == NULL) {
		SCLoggerLog(S_logger, LOG_ERR,
			    CFSTR("<%p> _dnsinfo_copy: xpc_dictionary_create_reply: failed"),
			    connection);
		return;
	}

	// extract data and generation #
	data = _libSC_info_server_get_data(&S_dns_info,
					   connection,
					   &generation);

	if (*S_debug) {
		const char	*proc_name;

		// extract process name
		proc_name = xpc_dictionary_get_string(request, DNSINFO_PROC_NAME);
		if (proc_name == NULL) {
			proc_name = "???";
		}

		SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("<%p:%s[%d]> DNS configuration copy: %llu"),
			    connection,
			    proc_name,
			    xpc_connection_get_pid(connection),
			    generation);
	}

	// return the DNS configuration (if available)
	if (data != NULL) {
		xpc_dictionary_set_data(reply,
					DNSINFO_CONFIGURATION,
					CFDataGetBytePtr(data),
					CFDataGetLength(data));
	}

	// reply
	xpc_connection_send_message(remote, reply);
	xpc_release(reply);

	return;
}


/*
 * _dnsinfo_acknowledge
 *
 * Called when a client wants to acknowledge processing
 * of the DNS configuration
 *
 * - caller must be running on the _dns_server_queue()
 */
static void
_dnsinfo_acknowledge(xpc_connection_t connection, xpc_object_t request)
{
	uint64_t	generation;
	Boolean		inSync;

	generation = xpc_dictionary_get_uint64(request, DNSINFO_GENERATION);

	if (*S_debug) {
		SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("<%p:%d> DNS configuration ack: %llu"),
			    connection,
			    xpc_connection_get_pid(connection),
			    generation);
	}

	(void) _libSC_info_server_acknowledged(&S_dns_info, connection, generation);

	// Note: all of the mDNSResponder ack's should result
	//       in a [new] network change being posted

	inSync = _libSC_info_server_in_sync(&S_dns_info);
	if (S_sync_handler) {
	    S_sync_handler(inSync);
	}
	return;
}


static void
process_request(xpc_connection_t connection, xpc_object_t request)
{
	int64_t		op;

	op = xpc_dictionary_get_int64(request, DNSINFO_REQUEST);
	switch (op) {
		case DNSINFO_REQUEST_COPY :
			/*
			 * Return the DNS configuration
			 */
			_dnsinfo_copy(connection, request);
			break;

		case DNSINFO_REQUEST_ACKNOWLEDGE :
			/*
			 * Acknowlege a [processed] DNS configuration
			 */
			_dnsinfo_acknowledge(connection, request);

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
		SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("<%p:%d> DNS configuration session: open"),
			    c,
			    xpc_connection_get_pid(c));
	}

	_libSC_info_server_open(&S_dns_info, c);

	xpc_connection_set_target_queue(c, _dnsinfo_server_queue());

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
					SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("<%p:%d> DNS configuration session: close"),
						    c,
						    xpc_connection_get_pid(c));
				}

				changed = _libSC_info_server_close(&S_dns_info, c);
				if (changed) {
					Boolean		inSync;

					// report change
					inSync = _libSC_info_server_in_sync(&S_dns_info);
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
#pragma mark DNS configuration server SPIs


__private_extern__
void
load_DNSConfiguration(CFBundleRef		bundle,
		      SCLoggerRef		logger,
		      Boolean			*bundleVerbose,
		      _dns_sync_handler_t	syncHandler)
{
	xpc_connection_t	c;
	const char		*name;

	S_debug = bundleVerbose;
	S_logger = logger;

	/*
	 * keep track of DNS configuration acknowledgements
	 */
	_libSC_info_server_init(&S_dns_info);

	/*
	 * save the in-sync/not-in-sync handler
	 */
	S_sync_handler = Block_copy(syncHandler);

	// create XPC listener
	name = getenv(DNSINFO_SERVICE_NAME);
	if (name == NULL) {
		name = DNSINFO_SERVICE_NAME;
	}

	c = xpc_connection_create_mach_service(name,
					       _dnsinfo_server_queue(),
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
				SCLoggerLog(S_logger, LOG_ERR, CFSTR("DNS configuration server: %s"), desc);
				xpc_release(c);
			} else if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
				SCLoggerLog(S_logger, LOG_ERR, CFSTR("DNS configuration server: %s"), desc);
			} else {
				SCLoggerLog(S_logger, LOG_ERR,
				      CFSTR("DNS configuration server: Connection error: %p : %s"),
				      event,
				      desc);
			}

		} else {
			SCLoggerLog(S_logger, LOG_ERR,
				    CFSTR("DNS configuration server: unknown event type : %p"),
				    type);

		}
	});

	xpc_connection_resume(c);

SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("XPC server \"%s\" started"), name);

	return;
}


__private_extern__
_Bool
_dns_configuration_store(dns_create_config_t *_config)
{
	_dns_config_buf_t	*config		= (_dns_config_buf_t *)*_config;
	Boolean			in_sync;
	uint64_t		new_generation	= 0;
	CFDataRef		new_dns_info	= NULL;
	const char		*notify_key;

	if (config != NULL) {
		const UInt8	*bytes;
		CFIndex		len;

		new_generation = config->config.generation;

		if (*S_debug) {
			SCLoggerLog(S_logger, LOG_DEBUG, CFSTR("DNS configuration updated: %llu"),
				    new_generation);
		}

		bytes = (const UInt8 *)config;
		len = sizeof(_dns_config_buf_t) + ntohl(config->n_attribute);

		new_dns_info = CFDataCreate(NULL, bytes, len);
	}

	dispatch_sync(_dnsinfo_server_queue(), ^{
		_libSC_info_server_set_data(&S_dns_info, new_dns_info, new_generation);
	});

	if (new_dns_info != NULL) {
		CFRelease(new_dns_info);
	}

	// if anyone is keeping us in sync, they now need to catch up
	in_sync = _libSC_info_server_in_sync(&S_dns_info);
	if (S_sync_handler) {
	    S_sync_handler(in_sync);
	}

	// and let everyone else know that the configuration has been updated
	notify_key = dns_configuration_notify_key();
	if (notify_key != NULL) {
		uint32_t	status;

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

	load_DNSConfiguration(CFBundleGetMainBundle(),		// bundle
			      NULL,				//SCLogger
			      &verbose,				// bundleVerbose
			      ^(Boolean inSync) {		// sync handler
				      SCLoggerLog(NULL, LOG_INFO,
					    CFSTR("in sync: %s"),
					    inSync ? "Yes" : "No")
			      });
	CFRunLoopRun();
	/* not reached */
	exit(0);
	return 0;
}

#endif  /* MAIN */
