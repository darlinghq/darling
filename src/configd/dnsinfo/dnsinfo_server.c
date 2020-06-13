/*
 * Copyright (c) 2004-2008, 2011-2017 Apple Inc. All rights reserved.
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
#include <bsm/libbsm.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>
#include <os/state_private.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCPrivate.h>

#include "libSystemConfiguration_client.h"
#include "libSystemConfiguration_server.h"

#include "dnsinfo_create.h"
#include "dnsinfo_server.h"
#include "dnsinfo_private.h"

#ifdef	SC_LOG_HANDLE
#include <os/log.h>
os_log_t	SC_LOG_HANDLE(void);
#endif	//SC_LOG_HANDLE


#pragma mark -
#pragma mark Globals


/*
 * S_dns_info
 *
 * Note: all accesses should be made while running on the _dns_server_queue()
 */
static libSC_info_server_t	S_dns_info;


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
	const char		*proc_name;
	xpc_connection_t	remote;
	xpc_object_t		reply;

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);
	if (reply == NULL) {
		SC_log(LOG_ERR, "<%p> _dnsinfo_copy: xpc_dictionary_create_reply: failed",
		       connection);
		return;
	}

	// extract data and generation #
	data = _libSC_info_server_get_data(&S_dns_info,
					   connection,
					   &generation);

	// extract process name
	proc_name = xpc_dictionary_get_string(request, DNSINFO_PROC_NAME);
	if (proc_name == NULL) {
		proc_name = "???";
	}

	SC_log(LOG_DEBUG, "<%p:%s[%d]> DNS configuration copy: %llu",
	       connection,
	       proc_name,
	       xpc_connection_get_pid(connection),
	       generation);

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

	SC_log(LOG_DEBUG, "<%p:%d> DNS configuration ack: %llu",
	       connection,
	       xpc_connection_get_pid(connection),
	       generation);

	(void) _libSC_info_server_acknowledged(&S_dns_info, connection, generation);

	// Note: all of the DNS service ack's should result
	//       in a [new] network change being posted

	inSync = _libSC_info_server_in_sync(&S_dns_info);
	if (S_sync_handler != NULL) {
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
			SC_log(LOG_ERR, "<%p> unknown request : %lld",
			       connection,
			       op);

			break;
	}

	return;
}


static void
process_new_connection(xpc_connection_t c)
{
	SC_log(LOG_DEBUG, "<%p:%d> DNS configuration session: open",
	       c,
	       xpc_connection_get_pid(c));

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

				SC_log(LOG_DEBUG, "<%p:%d> DNS configuration session: close",
				       c,
				       xpc_connection_get_pid(c));

				changed = _libSC_info_server_close(&S_dns_info, c);
				if (changed) {
					Boolean		inSync;

					// report change
					inSync = _libSC_info_server_in_sync(&S_dns_info);
					if (S_sync_handler != NULL) {
						S_sync_handler(inSync);
					}
				}

			} else if (xobj == XPC_ERROR_CONNECTION_INTERRUPTED) {
				SC_log(LOG_ERR, "<%p:%d> %s",
				       c,
				       xpc_connection_get_pid(c),
				       desc);

			} else {
				SC_log(LOG_ERR, "<%p:%d> Connection error: %p : %s",
				       c,
				       xpc_connection_get_pid(c),
				       xobj,
				       desc);
			}

		}  else {
			SC_log(LOG_ERR, "<%p:%d> unknown event type : %p",
			       c,
			       xpc_connection_get_pid(c),
			       type);
		}
	});

	xpc_connection_resume(c);

	return;
}


#pragma mark -
#pragma mark DNS configuration state


static void
add_state_handler()
{
#if	!TARGET_OS_SIMULATOR
	os_state_block_t	state_block;
	os_state_handle_t	state_handle;

	state_block = ^os_state_data_t(os_state_hints_t hints) {
#pragma unused(hints)
		CFIndex		dnsinfo_len;
		os_state_data_t	state_data;
		size_t		state_data_size;

		dnsinfo_len = (S_dns_info.data != NULL) ? CFDataGetLength(S_dns_info.data) : 0;
		state_data_size = OS_STATE_DATA_SIZE_NEEDED(dnsinfo_len);
		if (state_data_size > MAX_STATEDUMP_SIZE) {
			SC_log(LOG_ERR, "DNS configuration: state data too large (%zd > %zd)",
			       state_data_size,
			       (size_t)MAX_STATEDUMP_SIZE);
			return NULL;
		}

		state_data = calloc(1, state_data_size);
		if (state_data == NULL) {
			SC_log(LOG_ERR, "DNS configuration: could not allocate state data");
			return NULL;
		}

		state_data->osd_type = OS_STATE_DATA_CUSTOM;
		state_data->osd_data_size = (uint32_t)dnsinfo_len;
		strlcpy(state_data->osd_decoder.osdd_library,
			"SystemConfiguration",
			sizeof(state_data->osd_decoder.osdd_library));
		strlcpy(state_data->osd_decoder.osdd_type,
			"dnsinfo",
			sizeof(state_data->osd_decoder.osdd_type));
		strlcpy(state_data->osd_title, "DNS Configuration", sizeof(state_data->osd_title));
		if (dnsinfo_len > 0) {
			memcpy(state_data->osd_data, CFDataGetBytePtr(S_dns_info.data), dnsinfo_len);
		}

		return state_data;
	};

	state_handle = os_state_add_handler(_dnsinfo_server_queue(), state_block);
	if (state_handle == 0) {
		SC_log(LOG_ERR, "DNS configuration: os_state_add_handler() failed");
	}
#endif	// !TARGET_OS_SIMULATOR


	return;
}


#pragma mark -
#pragma mark DNS configuration server SPIs


__private_extern__
void
load_DNSConfiguration(CFBundleRef		bundle,
		      _dns_sync_handler_t	syncHandler)
{
#pragma unused(bundle)
	xpc_connection_t	c;
	const char		*name;

	/*
	 * keep track of DNS configuration acknowledgements
	 */
	_libSC_info_server_init(&S_dns_info);

	/*
	 * add a state dump handler
	 */
	add_state_handler();

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
				SC_log(LOG_ERR, "DNS configuration server: %s", desc);
				xpc_release(c);
			} else if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
				SC_log(LOG_ERR, "DNS configuration server: %s", desc);
			} else {
				SC_log(LOG_ERR, "DNS configuration server: Connection error: %p : %s",
				       event,
				       desc);
			}

		} else {
			SC_log(LOG_ERR, "DNS configuration server: unknown event type : %p", type);
		}
	});

	xpc_connection_resume(c);

SC_log(LOG_DEBUG, "XPC server \"%s\" started", name);

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

		SC_log(LOG_INFO, "DNS configuration updated: %llu",
		       new_generation);

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
	if (S_sync_handler != NULL) {
	    S_sync_handler(in_sync);
	}

	// and let everyone else know that the configuration has been updated
	notify_key = dns_configuration_notify_key();
	if (notify_key != NULL) {
		uint32_t	status;

		status = notify_post(notify_key);
		if (status != NOTIFY_STATUS_OK) {
			SC_log(LOG_ERR, "notify_post() failed: %d", status);
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
			      ^(Boolean inSync) {		// sync handler
				      SC_log(LOG_INFO,
					     "in sync: %s",
					     inSync ? "Yes" : "No")
			      });
	CFRunLoopRun();
	/* not reached */
	exit(0);
	return 0;
}

#endif  /* MAIN */
