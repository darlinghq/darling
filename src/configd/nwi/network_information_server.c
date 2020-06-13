/*
 * Copyright (c) 2012-2018 Apple Inc. All rights reserved.
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
#include <os/state_private.h>
#include <CommonCrypto/CommonDigest.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCPrivate.h>

#include "libSystemConfiguration_client.h"
#include "libSystemConfiguration_server.h"

#include <network_information.h>
#include "network_information_internal.h"
#include "network_information_server.h"
#include "network_state_information_priv.h"

#if	!TARGET_OS_SIMULATOR
#include "agent-monitor.h"
#include "configAgentDefines.h"
#include "network_config_agent_info_priv.h"
#endif	// !TARGET_OS_SIMULATOR

#ifdef	SC_LOG_HANDLE
#include <os/log.h>
os_log_t	SC_LOG_HANDLE(void);
#endif	//SC_LOG_HANDLE


#pragma mark -
#pragma mark Globals


/*
 * S_nwi_info
 *
 * Note: all accesses should be made while running on the _nwi_server_queue()
 */
static libSC_info_server_t	S_nwi_info;


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
	const char		*proc_name;
	xpc_connection_t	remote;
	xpc_object_t		reply;

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);
	if (reply == NULL) {
		SC_log(LOG_ERR, "<%p> _nwi_state_copy: xpc_dictionary_create_reply: failed",
		       connection);
		return;
	}

	// extract data and generation #
	data = _libSC_info_server_get_data(&S_nwi_info, connection, &generation);

	// extract process name
	proc_name = xpc_dictionary_get_string(request, NWI_PROC_NAME);
	if (proc_name == NULL) {
		proc_name = "???";
	}

	SC_log(LOG_DEBUG, "<%p:%s[%d]> Network information copy: %llu",
	       connection,
	       proc_name,
	       xpc_connection_get_pid(connection),
	       generation);

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

	SC_log(LOG_DEBUG, "<%p:%d> Network information ack: %llu",
	       connection,
	       xpc_connection_get_pid(connection),
	       generation);

	changed = _libSC_info_server_acknowledged(&S_nwi_info, connection, generation);
	if (changed) {
		Boolean		inSync;

		// report change
		inSync = _libSC_info_server_in_sync(&S_nwi_info);
		if (S_sync_handler != NULL) {
			S_sync_handler(inSync);
		}
	}

	return;
}

#if	!TARGET_OS_SIMULATOR
/*
 * _nwi_config_agent_copy
 *
 * Called when a client wants a copy of the agent data
 *
 * - caller must be running on the _nwi_server_queue()
 */
static void
_nwi_config_agent_copy(xpc_connection_t connection, xpc_object_t request)
{
	const void *buffer = NULL;
	const char *proc_name = NULL;
	uint64_t length = 0;
	xpc_connection_t	remote;
	xpc_object_t		reply = NULL;

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);

	uuid_t agent_uuid;
	const uint8_t *agent_uuid_value = xpc_dictionary_get_uuid(request, kConfigAgentAgentUUID);
	if (agent_uuid_value != NULL) {
		uuid_copy(agent_uuid, agent_uuid_value);
	} else {
		goto done;
	}

	const char *agent_type = xpc_dictionary_get_string(request, kConfigAgentType);
	if (agent_type == NULL) {
		goto done;
	}

	proc_name = xpc_dictionary_get_string(request, NWI_PROC_NAME);
	if (proc_name == NULL) {
		proc_name = "???";
	}

	SC_log(LOG_DEBUG, "<%p:%s[%d]> Config agent information copy",
	       connection,
	       proc_name,
	       xpc_connection_get_pid(connection));

	if (strcmp(agent_type, kConfigAgentTypeDNS) == 0) {
		buffer = copy_dns_information_for_agent_uuid(agent_uuid, &length);
	} else if (strcmp(agent_type, kConfigAgentTypeProxy) == 0) {
		buffer = copy_proxy_information_for_agent_uuid(agent_uuid, &length);
	}

	if (buffer != NULL && length > 0) {
		xpc_dictionary_set_data(reply,
					kConfigAgentAgentData,
					buffer,
					(size_t)length);
	}

	xpc_connection_send_message(remote, reply);

done:
	if (reply != NULL) {
		xpc_release(reply);
	}

	if (buffer != NULL) {
		free((void *)buffer);
	}

	return;
}
#endif	// !TARGET_OS_SIMULATOR


static void
process_request(xpc_connection_t connection, xpc_object_t request)
{
	int64_t		op;

	op = xpc_dictionary_get_int64(request, NWI_REQUEST);
	switch (op) {
		case NWI_STATE_REQUEST_COPY :
			/*
			 * Return the Network information
			 */
			_nwi_state_copy(connection, request);
			break;

		case NWI_STATE_REQUEST_ACKNOWLEDGE :
			/*
			 * Acknowlege a [processed] Network information
			 */
			_nwi_state_acknowledge(connection, request);

			break;
#if	!TARGET_OS_SIMULATOR
		case NWI_CONFIG_AGENT_REQUEST_COPY :
			/*
			 * Return the agent information
			 */
			_nwi_config_agent_copy(connection, request);

			break;
#endif	// !TARGET_OS_SIMULATOR
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
	SC_log(LOG_DEBUG, "<%p:%d> Network information session: open",
	       c,
	       xpc_connection_get_pid(c));

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

				SC_log(LOG_DEBUG, "<%p:%d> Network information session: close",
				       c,
				       xpc_connection_get_pid(c));

				changed = _libSC_info_server_close(&S_nwi_info, c);
				if (changed) {
					Boolean		inSync;

					// report change
					inSync = _libSC_info_server_in_sync(&S_nwi_info);
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
#pragma mark Network information state


static void
add_state_handler()
{
#if	!TARGET_OS_SIMULATOR
	os_state_block_t	state_block;
	os_state_handle_t	state_handle;

	state_block = ^os_state_data_t(os_state_hints_t hints) {
#pragma unused(hints)
		os_state_data_t	state_data;
		size_t		state_data_size;
		CFIndex		state_len;

		state_len = (S_nwi_info.data != NULL) ? CFDataGetLength(S_nwi_info.data) : 0;
		state_data_size = OS_STATE_DATA_SIZE_NEEDED(state_len);
		if (state_data_size > MAX_STATEDUMP_SIZE) {
			SC_log(LOG_ERR, "Network information : state data too large (%zd > %zd)",
			       state_data_size,
			       (size_t)MAX_STATEDUMP_SIZE);
			return NULL;
		}

		state_data = calloc(1, state_data_size);
		if (state_data == NULL) {
			SC_log(LOG_ERR, "Network information: could not allocate state data");
			return NULL;
		}

		state_data->osd_type = OS_STATE_DATA_CUSTOM;
		state_data->osd_data_size = (uint32_t)state_len;
		strlcpy(state_data->osd_decoder.osdd_library,
			"SystemConfiguration",
			sizeof(state_data->osd_decoder.osdd_library));
		strlcpy(state_data->osd_decoder.osdd_type,
			"nwi",
			sizeof(state_data->osd_decoder.osdd_type));
		strlcpy(state_data->osd_title, "Network information", sizeof(state_data->osd_title));
		if (state_len > 0) {
			memcpy(state_data->osd_data, CFDataGetBytePtr(S_nwi_info.data), state_len);
		}

		return state_data;
	};

	state_handle = os_state_add_handler(_nwi_state_server_queue(), state_block);
	if (state_handle == 0) {
		SC_log(LOG_ERR, "Network information: os_state_add_handler() failed");
	}
#endif	// !TARGET_OS_SIMULATOR


	return;
}


#pragma mark -
#pragma mark Network information server SPIs


__private_extern__
void
load_NetworkInformation(CFBundleRef		bundle,
			_nwi_sync_handler_t	syncHandler)
{
#pragma unused(bundle)
	xpc_connection_t	c;
	const char		*name;

	/*
	 * keep track of Network information acknowledgements
	 */
	_libSC_info_server_init(&S_nwi_info);

	/*
	 * add a state dump handler
	 */
	add_state_handler();

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
				SC_log(LOG_ERR, "Network information server: %s", desc);
				xpc_release(c);
			} else if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
				SC_log(LOG_ERR, "Network information server: %s", desc);
			} else {
				SC_log(LOG_ERR, "Network information server: Connection error: %p : %s",
				       event,
				       desc);
			}

		} else {
			SC_log(LOG_ERR, "Network information server: unknown event type : %p", type);

		}
	});

	xpc_connection_resume(c);

SC_log(LOG_DEBUG, "XPC server \"%s\" started", name);

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

		SC_log(LOG_DEBUG, "Network information updated: %llu",
		       new_generation);

		bytes = (const UInt8 *)state;
		len = nwi_state_size(state);

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
	if (S_sync_handler != NULL) {
		S_sync_handler(in_sync);
	}

	// and let everyone else know that the configuration has been updated
	notify_key = nwi_state_get_notify_key();
	if (notify_key != NULL) {
		uint32_t	status;

		_nwi_state_force_refresh();
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

	load_NetworkInformation(CFBundleGetMainBundle(),	// bundle
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

#endif	/* MAIN */
