/*
 * Copyright (c) 2011-2014 Apple Inc. All rights reserved.
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

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCNetworkReachabilityInternal.h"

#include <xpc/xpc.h>
#include <xpc/private.h>
#include <sys/rbtree.h>


#pragma mark -
#pragma mark Globals


static Boolean			serverAvailable	= TRUE;


#pragma mark -
#pragma mark Support functions


static void
log_xpc_object(const char *msg, xpc_object_t obj)
{
	char		*desc;

	desc = xpc_copy_description(obj);
	SCLog(TRUE, LOG_DEBUG, CFSTR("%s = %s"), msg, desc);
	free(desc);
}


#pragma mark -
#pragma mark Reachability [RBT] client support


typedef struct {
	rb_node_t			rbn;
	SCNetworkReachabilityRef	target;
} reach_request_t;


static int
_rbt_compare_transaction_nodes(void *context, const void *n1, const void *n2)
{
	uint64_t	a = (uintptr_t)(((reach_request_t *)n1)->target);
	uint64_t	b = (uintptr_t)(((reach_request_t *)n2)->target);

	if (a == b) {
		return 0;
	} else if (a < b) {
		return -1;
	} else {
		return 1;
	}
}


static int
_rbt_compare_transaction_key(void *context, const void *n1, const void *key)
{
	uint64_t	a = (uintptr_t)(((reach_request_t *)n1)->target);
	uint64_t	b = *(uint64_t *)key;

	if (a == b) {
		return 0;
	} else if (a < b) {
		return -1;
	} else {
		return 1;
	}
}


static rb_tree_t *
_reach_requests_rbt()
{
	static dispatch_once_t		once;
	static const rb_tree_ops_t	ops = {
		.rbto_compare_nodes	= _rbt_compare_transaction_nodes,
		.rbto_compare_key	= _rbt_compare_transaction_key,
		.rbto_node_offset	= offsetof(reach_request_t, rbn),
		.rbto_context		= NULL
	};
	static rb_tree_t		rbt;

	dispatch_once(&once, ^{
		rb_tree_init(&rbt, &ops);
	});

	return &rbt;
}


static dispatch_queue_t
_reach_requests_rbt_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(REACH_SERVICE_NAME ".requests.rbt", NULL);
	});

	return q;
}


static reach_request_t *
_reach_request_create(SCNetworkReachabilityRef target)
{
	reach_request_t	*request;

	request = calloc(1, sizeof(*request));
	request->target = CFRetain(target);

	return request;
}


static void
_reach_request_release(reach_request_t *request)
{
	SCNetworkReachabilityRef	target	= request->target;

	CFRelease(target);
	free(request);

	return;
}


static void
_reach_request_add(SCNetworkReachabilityRef target)
{
	uint64_t	target_id	= (uintptr_t)target;

	dispatch_sync(_reach_requests_rbt_queue(), ^{
		rb_tree_t	*rbt	= _reach_requests_rbt();
		reach_request_t	*request;

		request = rb_tree_find_node(rbt, &target_id);
		if (request == NULL) {
			request = _reach_request_create(target);
			rb_tree_insert_node(rbt, request);
		}
	});

	return;
}


static void
_reach_request_remove(SCNetworkReachabilityRef target)
{
	uint64_t	target_id	= (uintptr_t)target;

	dispatch_sync(_reach_requests_rbt_queue(), ^{		// FIXME ?? use dispatch_async?
		rb_tree_t	*rbt	= _reach_requests_rbt();
		reach_request_t	*request;

		request = rb_tree_find_node(rbt, &target_id);
		if (request != NULL) {
			rb_tree_remove_node(rbt, request);
			_reach_request_release(request);
		}
	});

	return;
}


static SCNetworkReachabilityRef
_reach_request_copy_target(uint64_t target_id)
{
	__block SCNetworkReachabilityRef	target	= NULL;

	dispatch_sync(_reach_requests_rbt_queue(), ^{
		rb_tree_t	*rbt	= _reach_requests_rbt();
		reach_request_t	*request;

		request = rb_tree_find_node(rbt, &target_id);
		if (request != NULL) {
			target = request->target;
			CFRetain(target);
		}
	});

	return target;
}


#pragma mark -
#pragma mark Reachability [XPC] client support


static void
handle_reachability_status(SCNetworkReachabilityRef target, xpc_object_t dict)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (_sc_debug) {
		SCLog(TRUE, LOG_INFO, CFSTR("%sgot [async] notification"),
		      targetPrivate->log_prefix);
//		log_xpc_object("  status", dict);
	}

	__SCNetworkReachabilityUpdateConcurrent(target);

	return;
}


static void
handle_async_notification(SCNetworkReachabilityRef target, xpc_object_t dict)
{
	int64_t				op;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	op = xpc_dictionary_get_int64(dict, MESSAGE_NOTIFY);
	switch (op) {
		case MESSAGE_REACHABILITY_STATUS :
			handle_reachability_status(target, dict);
			break;
		default :
			SCLog(TRUE, LOG_ERR, CFSTR("%sgot [async] unknown reply : %lld"),
			      targetPrivate->log_prefix,
			      op);
			log_xpc_object("  reply", dict);
			break;
	}

	return;
}


static dispatch_queue_t
_reach_xpc_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(REACH_SERVICE_NAME ".xpc", NULL);
	});

	return q;
}


static void
_reach_connection_reconnect(xpc_connection_t connection);


static xpc_connection_t
_reach_connection_create()
{
	xpc_connection_t	c;
#if	!TARGET_IPHONE_SIMULATOR
	const uint64_t		flags	= XPC_CONNECTION_MACH_SERVICE_PRIVILEGED;
#else	// !TARGET_IPHONE_SIMULATOR
	const uint64_t		flags	= 0;
#endif	// !TARGET_IPHONE_SIMULATOR
	const char		*name;
	dispatch_queue_t	q	= _reach_xpc_queue();

	// create XPC connection
	name = getenv("REACH_SERVER");
	if ((name == NULL) || (issetugid() != 0)) {
		name = REACH_SERVICE_NAME;
	}

	c = xpc_connection_create_mach_service(name, q, flags);

	xpc_connection_set_event_handler(c, ^(xpc_object_t xobj) {
		xpc_type_t	type;

		type = xpc_get_type(xobj);
		if (type == XPC_TYPE_DICTIONARY) {
			SCNetworkReachabilityRef	target;
			uint64_t			target_id;

			target_id = xpc_dictionary_get_uint64(xobj, REACH_CLIENT_TARGET_ID);
			if (target_id == 0) {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("reach client %p: async reply with no target [ID]"),
				      c);
				log_xpc_object("  reply", xobj);
				return;
			}

			target = _reach_request_copy_target(target_id);
			if (target == NULL) {
//				SCLog(TRUE, LOG_ERR,
//				      CFSTR("received unexpected target [ID] from SCNetworkReachability server"));
//				log_xpc_object("  reply", xobj);
				return;
			}

			xpc_retain(xobj);
			dispatch_async(__SCNetworkReachability_concurrent_queue(), ^{
				handle_async_notification(target, xobj);
				CFRelease(target);
				xpc_release(xobj);
			});

		} else if (type == XPC_TYPE_ERROR) {
			if (xobj == XPC_ERROR_CONNECTION_INVALID) {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("SCNetworkReachability server not available"));
				serverAvailable = FALSE;
			} else if (xobj == XPC_ERROR_CONNECTION_INTERRUPTED) {
				SCLog(TRUE, LOG_DEBUG,
				      CFSTR("SCNetworkReachability server failure, reconnecting"));
				_reach_connection_reconnect(c);
			} else {
				const char	*desc;

				desc = xpc_dictionary_get_string(xobj, XPC_ERROR_KEY_DESCRIPTION);
				SCLog(TRUE, LOG_ERR,
				      CFSTR("reach client %p: Connection error: %s"),
				      c,
				      desc);
			}

		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("reach client %p: unknown event type : %p"),
			      c,
			      type);
		}
	});
	xpc_connection_resume(c);

	return c;
}


static xpc_connection_t
_reach_connection()
{
	static xpc_connection_t		c;
	static dispatch_once_t		once;
	static dispatch_queue_t		q;

	if (!serverAvailable) {
		// if SCNetworkReachability [XPC] server not available
		return NULL;
	}

	dispatch_once(&once, ^{
		q = dispatch_queue_create(REACH_SERVICE_NAME ".connection", NULL);
	});

	dispatch_sync(q, ^{
		if (c == NULL) {
			c = _reach_connection_create();
		}
	});

	return c;
}


typedef void (^reach_server_reply_handler_t)(xpc_object_t reply);


static void
add_proc_name(xpc_object_t reqdict)
{
	static const char	*name	= NULL;
	static dispatch_once_t	once;

	// add the process name
	dispatch_once(&once, ^{
		name = getprogname();
	});
	xpc_dictionary_set_string(reqdict, REACH_CLIENT_PROC_NAME, name);

	return;
}


static void
_reach_server_target_reconnect(xpc_connection_t connection, SCNetworkReachabilityRef target, Boolean disconnect);


static Boolean
_reach_server_target_add(xpc_connection_t connection, SCNetworkReachabilityRef target)
{
	Boolean				ok		= FALSE;
	xpc_object_t			reply;
	xpc_object_t			reqdict;
	Boolean				retry		= FALSE;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	// create message
	reqdict = xpc_dictionary_create(NULL, NULL, 0);

	// set request
	xpc_dictionary_set_int64(reqdict, REACH_REQUEST, REACH_REQUEST_CREATE);

	// add reachability target info
	switch (targetPrivate->type) {
		case reachabilityTypeName :
			xpc_dictionary_set_string(reqdict,
						  REACH_TARGET_NAME,
						  targetPrivate->name);
			break;
		case reachabilityTypeAddress :
		case reachabilityTypeAddressPair :
			if (targetPrivate->localAddress != NULL) {
				xpc_dictionary_set_data(reqdict,
							REACH_TARGET_LOCAL_ADDR,
							targetPrivate->localAddress,
							targetPrivate->localAddress->sa_len);
			}
			if (targetPrivate->remoteAddress != NULL) {
				xpc_dictionary_set_data(reqdict,
							REACH_TARGET_REMOTE_ADDR,
							targetPrivate->remoteAddress,
							targetPrivate->remoteAddress->sa_len);
			}
			break;
		case reachabilityTypePTR :
			xpc_dictionary_set_data(reqdict,
						REACH_TARGET_PTR_ADDR,
						targetPrivate->remoteAddress,
						targetPrivate->remoteAddress->sa_len);
			break;
	}
	if (targetPrivate->if_index != 0) {
		xpc_dictionary_set_int64(reqdict,
					 REACH_TARGET_IF_INDEX,
					 targetPrivate->if_index);
		xpc_dictionary_set_string(reqdict,
					  REACH_TARGET_IF_NAME,
					  targetPrivate->if_name);
	}
	if (targetPrivate->onDemandBypass) {
		xpc_dictionary_set_bool(reqdict,
					REACH_TARGET_ONDEMAND_BYPASS,
					TRUE);
	}
	if (targetPrivate->resolverBypass) {
		xpc_dictionary_set_bool(reqdict,
					REACH_TARGET_RESOLVER_BYPASS,
					TRUE);
	}



	// add the target [ID]
	xpc_dictionary_set_uint64(reqdict, REACH_CLIENT_TARGET_ID, (uintptr_t)target);

	// add the process name (for debugging)
	add_proc_name(reqdict);

    retry :

	// send request to the SCNetworkReachability server
	reply = xpc_connection_send_message_with_reply_sync(connection, reqdict);
	if (reply != NULL) {
		xpc_type_t	type;

		type = xpc_get_type(reply);
		if (type == XPC_TYPE_DICTIONARY) {
			int64_t		status;

			status = xpc_dictionary_get_int64(reply, REACH_REQUEST_REPLY);
			ok = (status == REACH_REQUEST_REPLY_OK);
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INVALID)) {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("SCNetworkReachability server not available"));
			serverAvailable = FALSE;
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INTERRUPTED)) {
			SCLog(TRUE, LOG_DEBUG,
			      CFSTR("reach target %p: SCNetworkReachability server failure, retrying"),
			      target);
			retry = TRUE;
		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("reach target %p: _targetAdd with unexpected reply"),
			      target);
			log_xpc_object("  reply", reply);
		}

		xpc_release(reply);
	}

	if (retry) {
		retry = FALSE;
		goto retry;
	}

	xpc_release(reqdict);
	return ok;
}


static Boolean
_reach_server_target_remove(xpc_connection_t connection, SCNetworkReachabilityRef target)
{
	Boolean				ok		= FALSE;
	xpc_object_t			reply;
	xpc_object_t			reqdict;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	// create message
	reqdict = xpc_dictionary_create(NULL, NULL, 0);

	// set request
	xpc_dictionary_set_int64(reqdict, REACH_REQUEST, REACH_REQUEST_REMOVE);

	// add the target [ID]
	xpc_dictionary_set_uint64(reqdict, REACH_CLIENT_TARGET_ID, (uintptr_t)target);

	reply = xpc_connection_send_message_with_reply_sync(connection, reqdict);
	if (reply != NULL) {
		xpc_type_t	type;

		type = xpc_get_type(reply);
		if (type == XPC_TYPE_DICTIONARY) {
			int64_t		status;

			status = xpc_dictionary_get_int64(reply, REACH_REQUEST_REPLY);
			switch (status) {
				case REACH_REQUEST_REPLY_OK :
					ok = TRUE;
					break;
				case REACH_REQUEST_REPLY_UNKNOWN :
					// target not known by the server (most likely due to a
					// SCNetworkReachability server failure), no need to
					// remove.
					ok = TRUE;
					break;
				default : {
					SCLog(TRUE, LOG_ERR, CFSTR("%s  target remove failed"),
					      targetPrivate->log_prefix);
					log_xpc_object("  reply", reply);
				}
			}
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INVALID)) {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("SCNetworkReachability server not available"));
			serverAvailable = FALSE;
			ok = TRUE;
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INTERRUPTED)) {
			SCLog(TRUE, LOG_DEBUG,
			      CFSTR("reach target %p: SCNetworkReachability server failure, no need to remove"),
			      target);
			ok = TRUE;
		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("reach target %p: _targetRemove with unexpected reply"),
			      target);
			log_xpc_object("  reply", reply);
		}

		xpc_release(reply);
	}

	xpc_release(reqdict);
	return ok;
}


static Boolean
_reach_server_target_schedule(xpc_connection_t connection, SCNetworkReachabilityRef target)
{
	Boolean				ok		= FALSE;
	xpc_object_t			reply;
	xpc_object_t			reqdict;
	Boolean				retry		= FALSE;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	// create message
	reqdict = xpc_dictionary_create(NULL, NULL, 0);

	// set request
	xpc_dictionary_set_int64(reqdict, REACH_REQUEST, REACH_REQUEST_SCHEDULE);

	// add the target [ID]
	xpc_dictionary_set_uint64(reqdict, REACH_CLIENT_TARGET_ID, (uintptr_t)target);

    retry :

	reply = xpc_connection_send_message_with_reply_sync(connection, reqdict);
	if (reply != NULL) {
		xpc_type_t	type;

		type = xpc_get_type(reply);
		if (type == XPC_TYPE_DICTIONARY) {
			int64_t		status;

			status = xpc_dictionary_get_int64(reply, REACH_REQUEST_REPLY);
			switch (status) {
				case REACH_REQUEST_REPLY_OK :
					ok = TRUE;
					break;
				case REACH_REQUEST_REPLY_UNKNOWN :
					// target not known by the server (most likely due to a
					// SCNetworkReachability server failure), re-establish
					// and retry scheduling.
					retry = TRUE;
					break;
				default : {
					SCLog(TRUE, LOG_ERR, CFSTR("%s  target schedule failed"),
					      targetPrivate->log_prefix);
					log_xpc_object("  reply", reply);
				}
			}

			if (ok) {
				CFRetain(target);
			}
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INVALID)) {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("SCNetworkReachability server not available"));
			serverAvailable = FALSE;
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INTERRUPTED)) {
			SCLog(TRUE, LOG_DEBUG,
			      CFSTR("reach target %p: SCNetworkReachability server failure, retry schedule"),
			      target);
			retry = TRUE;
		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("reach target %p: _targetSchedule with unexpected reply"),
			      target);
			log_xpc_object("  reply", reply);
		}

		xpc_release(reply);
	}

	if (retry) {
		// reconnect
		_reach_server_target_reconnect(connection, target, FALSE);

		// and retry
		retry = FALSE;
		goto retry;
	}

	xpc_release(reqdict);
	return ok;
}


static void
_reach_reply_set_reachability(SCNetworkReachabilityRef	target,
			      xpc_object_t		reply)
{
	char	*if_name;
	size_t	len		= 0;

	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	targetPrivate->serverInfo.cycle = xpc_dictionary_get_uint64(reply,
								    REACH_STATUS_CYCLE);

	targetPrivate->serverInfo.flags = (SCNetworkReachabilityFlags)xpc_dictionary_get_uint64(reply,
												REACH_STATUS_FLAGS);

	targetPrivate->serverInfo.if_index = (unsigned int)xpc_dictionary_get_uint64(reply,
										     REACH_STATUS_IF_INDEX);

	bzero(&targetPrivate->serverInfo.if_name, sizeof(targetPrivate->serverInfo.if_name));
	if_name = (void *)xpc_dictionary_get_data(reply,
						  REACH_STATUS_IF_NAME,
						  &len);
	if ((if_name != NULL) && (len > 0)) {
		if (len > sizeof(targetPrivate->serverInfo.if_name)) {
			len = sizeof(targetPrivate->serverInfo.if_name);
		}

		bcopy(if_name, targetPrivate->serverInfo.if_name, len);
	}

	targetPrivate->serverInfo.sleeping = xpc_dictionary_get_bool(reply,
								     REACH_STATUS_SLEEPING);

	if (isReachabilityTypeName(targetPrivate->type)) {
		xpc_object_t		addresses;

		if (targetPrivate->resolvedAddresses != NULL) {
			CFRelease(targetPrivate->resolvedAddresses);
			targetPrivate->resolvedAddresses = NULL;
		}

		targetPrivate->resolvedError = (int)xpc_dictionary_get_int64(reply,
									     REACH_STATUS_RESOLVED_ERROR);

		addresses = xpc_dictionary_get_value(reply, REACH_STATUS_RESOLVED_ADDRESSES);
		if ((addresses != NULL) && (xpc_get_type(addresses) != XPC_TYPE_ARRAY)) {
			addresses = NULL;
		}

		if ((targetPrivate->resolvedError == NETDB_SUCCESS) && (addresses != NULL)) {
			int			i;
			size_t			n;
			CFMutableArrayRef	newAddresses;

			newAddresses = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

			n = xpc_array_get_count(addresses);
			for (i = 0; i < n; i++) {
				if (targetPrivate->type == reachabilityTypeName) {
					struct sockaddr	*sa;
					size_t		len;
					CFDataRef	newAddress;

					sa = (struct sockaddr *)xpc_array_get_data(addresses, i, &len);
					newAddress = CFDataCreate(NULL, (const UInt8 *)sa, len);
					CFArrayAppendValue(newAddresses, newAddress);
					CFRelease(newAddress);
				} else if (targetPrivate->type == reachabilityTypePTR) {
					const char	*str;
					CFStringRef	newName;

					str = xpc_array_get_string(addresses, i);
					newName = CFStringCreateWithCString(NULL, str, kCFStringEncodingUTF8);
					CFArrayAppendValue(newAddresses, newName);
					CFRelease(newName);
				}
			}

			targetPrivate->resolvedAddresses = newAddresses;
		} else {
			/* save the error associated with the attempt to resolve the name */
			targetPrivate->resolvedAddresses = CFRetain(kCFNull);
		}

		targetPrivate->dnsFlags = (uint32_t)xpc_dictionary_get_uint64(reply,
									      REACH_STATUS_DNS_FLAGS);

		targetPrivate->needResolve = FALSE;
	}

	return;
}


static Boolean
_reach_server_target_status(xpc_connection_t connection, SCNetworkReachabilityRef target)
{
	Boolean				ok		= FALSE;
	xpc_object_t			reply;
	xpc_object_t			reqdict;
	Boolean				retry		= FALSE;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (_sc_debug) {
		CFStringRef	str;

		str = _SCNetworkReachabilityCopyTargetDescription(target);
		SCLog(TRUE, LOG_INFO, CFSTR("%scheckReachability(%@)"),
		      targetPrivate->log_prefix,
		      str);
		CFRelease(str);
	}

	// create message
	reqdict = xpc_dictionary_create(NULL, NULL, 0);

	// set request
	xpc_dictionary_set_int64(reqdict, REACH_REQUEST, REACH_REQUEST_STATUS);

	// add the target [ID]
	xpc_dictionary_set_uint64(reqdict, REACH_CLIENT_TARGET_ID, (uintptr_t)target);

    retry :

	reply = xpc_connection_send_message_with_reply_sync(connection, reqdict);
	if (reply != NULL) {
		xpc_type_t	type;

		type = xpc_get_type(reply);
		if (type == XPC_TYPE_DICTIONARY) {
			int64_t		status;

			status = xpc_dictionary_get_int64(reply, REACH_REQUEST_REPLY);
			switch (status) {
				case REACH_REQUEST_REPLY_OK :
					ok = TRUE;
					break;
				case REACH_REQUEST_REPLY_UNKNOWN :
					// target not known by the server (most likely due to a
					// SCNetworkReachability server failure), re-establish
					// and retry status.
					retry = TRUE;
					break;
				default :
					SCLog(TRUE, LOG_INFO, CFSTR("%s  target status failed"),
					      targetPrivate->log_prefix);
					log_xpc_object("  reply", reply);
			}

			if (ok) {
				_reach_reply_set_reachability(target, reply);

				if (_sc_debug) {
					SCLog(TRUE, LOG_INFO, CFSTR("%s  flags     = 0x%08x"),
					      targetPrivate->log_prefix,
					      targetPrivate->serverInfo.flags);
					if (targetPrivate->serverInfo.if_index != 0) {
						SCLog(TRUE, LOG_INFO, CFSTR("%s  device    = %s (%u%s)"),
						      targetPrivate->log_prefix,
						      targetPrivate->serverInfo.if_name,
						      targetPrivate->serverInfo.if_index,
						      targetPrivate->serverInfo.sleeping ? ", z" : "");
					}
					if (targetPrivate->serverInfo.cycle != targetPrivate->cycle) {
						SCLog(TRUE, LOG_INFO, CFSTR("%s  forced"),
						      targetPrivate->log_prefix);
					}
				}
			}
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INVALID)) {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("SCNetworkReachability server not available"));
			serverAvailable = FALSE;
			ok = TRUE;
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INTERRUPTED)) {
			SCLog(TRUE, LOG_DEBUG,
			      CFSTR("reach target %p: SCNetworkReachability server failure, retry status"),
			      target);
			retry = TRUE;
		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("reach target %p: _targetStatus with unexpected reply"),
			      target);
			log_xpc_object("  reply", reply);
		}

		xpc_release(reply);
	}

	if (retry) {
		// reconnect
		_reach_server_target_reconnect(connection, target, FALSE);

		// and retry
		retry = FALSE;
		goto retry;
	}

	xpc_release(reqdict);
	return ok;
}


static Boolean
_reach_server_target_unschedule(xpc_connection_t connection, SCNetworkReachabilityRef target)
{
	Boolean				ok		= FALSE;
	xpc_object_t			reply;
	xpc_object_t			reqdict;
	Boolean				retry		= FALSE;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	// create message
	reqdict = xpc_dictionary_create(NULL, NULL, 0);

	// set request
	xpc_dictionary_set_int64(reqdict, REACH_REQUEST, REACH_REQUEST_UNSCHEDULE);

	// add the target [ID]
	xpc_dictionary_set_uint64(reqdict, REACH_CLIENT_TARGET_ID, (uintptr_t)target);

	reply = xpc_connection_send_message_with_reply_sync(connection, reqdict);
	if (reply != NULL) {
		xpc_type_t	type;

		type = xpc_get_type(reply);
		if (type == XPC_TYPE_DICTIONARY) {
			int64_t		status;

			status = xpc_dictionary_get_int64(reply, REACH_REQUEST_REPLY);
			switch (status) {
				case REACH_REQUEST_REPLY_OK :
					ok = TRUE;
					break;
				case REACH_REQUEST_REPLY_UNKNOWN :
					// target not known by the server (most likely due to a
					// SCNetworkReachability server failure), re-establish
					// but no need to unschedule.
					retry = TRUE;
					break;
				default :
					SCLog(TRUE, LOG_INFO, CFSTR("%s  target unschedule failed"),
					      targetPrivate->log_prefix);
					log_xpc_object("  reply", reply);
			}
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INVALID)) {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("SCNetworkReachability server not available"));
			serverAvailable = FALSE;
			ok = TRUE;
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INTERRUPTED)) {
			SCLog(TRUE, LOG_DEBUG,
			      CFSTR("reach target %p: SCNetworkReachability server failure, re-establish (but do not re-schedule)"),
			      target);
			retry = TRUE;
		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("reach target %p: _targetUnschedule with unexpected reply"),
			      target);
			log_xpc_object("  reply", reply);
		}

		xpc_release(reply);
	}

	if (retry) {
		// reconnect
		targetPrivate->serverScheduled = FALSE;
		_reach_server_target_reconnect(connection, target, FALSE);
		ok = TRUE;
	}

	if (ok) {
		CFRelease(target);
	}

	xpc_release(reqdict);
	return ok;
}


#pragma mark -
#pragma mark Reconnect


static void
_reach_server_target_reconnect(xpc_connection_t connection, SCNetworkReachabilityRef target, Boolean disconnect)
{
	Boolean				ok;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (!targetPrivate->serverActive) {
		// if target already removed
		return;
	}

	if (disconnect) {
		// if we should first disconnect (unschedule, remove)
		if (targetPrivate->serverScheduled) {
			(void) _reach_server_target_unschedule(connection, target);
		}

		(void) _reach_server_target_remove(connection, target);
	} else {
		// server has been restarted
		targetPrivate->cycle = 0;
	}

	// re-associate with server
	ok = _reach_server_target_add(connection, target);
	if (!ok) {
		// if we could not add the target
		return;
	}

	if (!targetPrivate->serverScheduled) {
		// if not scheduled
		return;
	}

	// ... and re-schedule with server
	ok = _reach_server_target_schedule(connection, target);
	if (!ok) {
		// if we could not reschedule the target
		return;
	}

	// For addresses, update our status now.  For names, queries will
	// be updated with a callback
	if (isReachabilityTypeAddress(targetPrivate->type)) {
		__SCNetworkReachabilityUpdate(target);
	}

	return;
}


static void
_reach_connection_reconnect(xpc_connection_t connection)
{
	dispatch_sync(_reach_requests_rbt_queue(), ^{
		rb_tree_t	*rbt	= _reach_requests_rbt();
		reach_request_t	*request;

		RB_TREE_FOREACH(request, rbt) {
			SCNetworkReachabilityRef	target;

			xpc_retain(connection);
			target = request->target;
			CFRetain(target);
			dispatch_async(__SCNetworkReachability_concurrent_queue(), ^{
				_reach_server_target_reconnect(connection, target, FALSE);
				CFRelease(target);
				xpc_release(connection);
			});
		}
	});

	return;
}


#pragma mark -
#pragma mark SPI (exposed)


Boolean
_SCNetworkReachabilityServer_snapshot(void)
{
	xpc_connection_t	c;
	Boolean			ok	= FALSE;
	xpc_object_t		reply;
	xpc_object_t		reqdict;

	// initialize connection with SCNetworkReachability server
	c = _reach_connection();
	if (c == NULL) {
		return FALSE;
	}

	// create message
	reqdict = xpc_dictionary_create(NULL, NULL, 0);

	// set request
	xpc_dictionary_set_int64(reqdict, REACH_REQUEST, REACH_REQUEST_SNAPSHOT);

	// add the process name (for debugging)
	add_proc_name(reqdict);

    retry :

	// send request
	reply = xpc_connection_send_message_with_reply_sync(c, reqdict);
	if (reply != NULL) {
		xpc_type_t	type;

		type = xpc_get_type(reply);
		if (type == XPC_TYPE_DICTIONARY) {
			int64_t		status;

			status = xpc_dictionary_get_int64(reply, REACH_REQUEST_REPLY);
			ok = (status == REACH_REQUEST_REPLY_OK);
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INVALID)) {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("SCNetworkReachability server not available"));
			serverAvailable = FALSE;
		} else if ((type == XPC_TYPE_ERROR) && (reply == XPC_ERROR_CONNECTION_INTERRUPTED)) {
			SCLog(TRUE, LOG_DEBUG,
			      CFSTR("SCNetworkReachability server failure, retrying"));
			xpc_release(reply);
			goto retry;
		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("_snapshot with unexpected reply"));
			log_xpc_object("  reply", reply);
		}

		xpc_release(reply);
	}

	xpc_release(reqdict);
	return ok;
}


__private_extern__
Boolean
__SCNetworkReachabilityServer_targetAdd(SCNetworkReachabilityRef target)
{
	xpc_connection_t		c;
	Boolean				ok;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	c = _reach_connection();
	if (c == NULL) {
		return FALSE;
	}

	ok = _reach_server_target_add(c, target);
	if (ok) {
		_SC_ATOMIC_CMPXCHG(&targetPrivate->serverActive, FALSE, TRUE);
	}

	return ok;
}


__private_extern__
void
__SCNetworkReachabilityServer_targetRemove(SCNetworkReachabilityRef target)
{
	xpc_connection_t		c;
	Boolean				ok;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (!targetPrivate->serverActive) {
		// if not active
		return;
	}

	c = _reach_connection();
	if (c == NULL) {
		return;
	}

	ok = _reach_server_target_remove(c, target);
	if (ok) {
		_SC_ATOMIC_CMPXCHG(&targetPrivate->serverActive, TRUE, FALSE);
	}

	return;
}


__private_extern__
Boolean
__SCNetworkReachabilityServer_targetSchedule(SCNetworkReachabilityRef target)
{
	xpc_connection_t		c;
	Boolean				ok;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	c = _reach_connection();
	if (c == NULL) {
		return FALSE;
	}

	_reach_request_add(target);
	ok = _reach_server_target_schedule(c, target);
	if (ok) {
		_SC_ATOMIC_CMPXCHG(&targetPrivate->serverScheduled, FALSE, TRUE);
	} else {
		_reach_request_remove(target);
	}

	return ok;
}


__private_extern__
Boolean
__SCNetworkReachabilityServer_targetStatus(SCNetworkReachabilityRef target)
{
	xpc_connection_t		c;
	Boolean				ok;

	c = _reach_connection();
	if (c == NULL) {
		return FALSE;
	}

	ok = _reach_server_target_status(c, target);
	return ok;
}


__private_extern__
Boolean
__SCNetworkReachabilityServer_targetUnschedule(SCNetworkReachabilityRef target)
{
	xpc_connection_t		c;
	Boolean				ok;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (!targetPrivate->serverScheduled) {
		// if not scheduled
		return TRUE;
	}

	c = _reach_connection();
	if (c == NULL) {
		return FALSE;
	}

	ok = _reach_server_target_unschedule(c, target);
	if (ok) {
		_SC_ATOMIC_CMPXCHG(&targetPrivate->serverScheduled, TRUE, FALSE);
		_reach_request_remove(target);
	} else {
		// if unschedule failed
	}

	return ok;
}


