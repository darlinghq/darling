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

#include <fcntl.h>
#include <paths.h>
#include <CommonCrypto/CommonDigest.h>
#include <dispatch/dispatch.h>
#include <dispatch/private.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <sys/rbtree.h>


#pragma mark -
#pragma mark Globals


/*
 * S_debug
 *   A boolean that enables additional logging.
 */
static boolean_t	S_debug		= FALSE;


#pragma mark -
#pragma mark Support functions


static void
log_xpc_object(const char *msg, xpc_object_t obj)
{
	char		*desc;

	desc = xpc_copy_description(obj);
	SCLog(S_debug, LOG_INFO, CFSTR("%s = %s"), msg, desc);
	free(desc);
}


static __inline__ void
my_CFDictionaryApplyFunction(CFDictionaryRef			theDict,
			     CFDictionaryApplierFunction	applier,
			     void				*context)
{
	CFAllocatorRef	myAllocator;
	CFDictionaryRef	myDict;

	myAllocator = CFGetAllocator(theDict);
	myDict      = CFDictionaryCreateCopy(myAllocator, theDict);
	CFDictionaryApplyFunction(myDict, applier, context);
	CFRelease(myDict);
	return;
}


#pragma mark -
#pragma mark SCNetworkReachability target support


static CFMutableDictionaryRef	reach_digest_map;


static dispatch_queue_t
_server_concurrent_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(REACH_SERVICE_NAME ".concurrent",
					  DISPATCH_QUEUE_CONCURRENT);
	});

	return q;
}


static dispatch_queue_t
_server_digest_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(REACH_SERVICE_NAME ".digest", NULL);
	});

	return q;
}


static dispatch_group_t
_target_group(SCNetworkReachabilityRef target)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	return targetPrivate->serverGroup;
}


static dispatch_queue_t
_target_queue(SCNetworkReachabilityRef target)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	return targetPrivate->serverQueue;
}


#pragma mark -


/*
 * _target_reference_add
 *
 * Note: use dispatch_sync(_server_digest_queue(), ^{ ... });
 */
static void
_target_reference_add(SCNetworkReachabilityRef target, CFDataRef digest, xpc_connection_t connection)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	// take a reference to the target
	CFRetain(target);

	// ensure that we have a dispatch group
	if (targetPrivate->serverGroup == NULL) {
		targetPrivate->serverGroup = dispatch_group_create();
	}

	// ensure that we have a dispatch queue
	if (targetPrivate->serverQueue == NULL) {
		char	qname[256];

		snprintf(qname, sizeof(qname), "com.apple.SCNetworkReachability.%p.server", target);
		targetPrivate->serverQueue = dispatch_queue_create(qname, NULL);
	}

	// bump the reference count
	if (_SC_ATOMIC_INC(&targetPrivate->serverReferences) == 0) {
		// and maintain a digest-->target mapping
		targetPrivate->serverDigest = CFRetain(digest);
		CFDictionarySetValue(reach_digest_map, digest, target);
	}

	if (S_debug) {
		CFStringRef	str;

		str = _SCNetworkReachabilityCopyTargetDescription(target);
		SCLog(TRUE, LOG_INFO,
		      CFSTR("<%p>   target %p: reference added (%@, %d)"),
		      connection,
		      target,
		      str,
		      targetPrivate->serverReferences);
		CFRelease(str);
	}

	return;
}


/*
 * _target_reference_remove
 *
 * Note: use dispatch_sync(_server_digest_queue(), ^{ ... });
 */
static void
_target_reference_remove(SCNetworkReachabilityRef target, xpc_connection_t connection)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	// drop the reference count
	if (_SC_ATOMIC_DEC(&targetPrivate->serverReferences) == 0) {
		/*
		 * if that was the last reference, we no longer need to
		 * keep the digest-->target mapping
		 */
		CFDictionaryRemoveValue(reach_digest_map, targetPrivate->serverDigest);
		CFRelease(targetPrivate->serverDigest);
		targetPrivate->serverDigest = NULL;
	}

	if (S_debug) {
		SCLog(TRUE, LOG_INFO,
		      CFSTR("<%p>   target %p: reference removed (%d)"),
		      connection,
		      target,
		      targetPrivate->serverReferences);
	}

	// release a reference to the target
	CFRelease(target);

	return;
}


#pragma mark -


#define	MUTEX_LOCK(m) {							\
	int _lock_ = (pthread_mutex_lock(m) == 0);			\
	assert(_lock_);							\
}

#define	MUTEX_UNLOCK(m) {						\
	int _unlock_ = (pthread_mutex_unlock(m) == 0);			\
	assert(_unlock_);						\
}


static void
_target_reply_add_reachability(SCNetworkReachabilityRef target,
			       xpc_object_t		reply)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	MUTEX_LOCK(&targetPrivate->lock);

	xpc_dictionary_set_uint64(reply,
				  REACH_STATUS_CYCLE,
				  targetPrivate->info.cycle);
	xpc_dictionary_set_uint64(reply,
				  REACH_STATUS_FLAGS,
				  targetPrivate->info.flags);
	xpc_dictionary_set_uint64(reply,
				  REACH_STATUS_IF_INDEX,
				  targetPrivate->info.if_index);
	xpc_dictionary_set_data  (reply,
				  REACH_STATUS_IF_NAME,
				  targetPrivate->info.if_name,
				  sizeof(targetPrivate->info.if_name));
	xpc_dictionary_set_bool	 (reply,
				  REACH_STATUS_SLEEPING,
				  targetPrivate->info.sleeping);
	if (isReachabilityTypeName(targetPrivate->type)) {
		if (isA_CFArray(targetPrivate->resolvedAddresses)) {
			xpc_object_t	addresses;
			CFIndex		i;
			CFIndex		n;

			addresses = xpc_array_create(NULL, 0);

			n = CFArrayGetCount(targetPrivate->resolvedAddresses);
			for (i = 0; i < n; i++) {
				if (targetPrivate->type == reachabilityTypeName) {
					CFDataRef	address;

					address = CFArrayGetValueAtIndex(targetPrivate->resolvedAddresses, i);
					xpc_array_set_data(addresses,
							   XPC_ARRAY_APPEND,
							   CFDataGetBytePtr(address),
							   CFDataGetLength(address));
				} else if (targetPrivate->type == reachabilityTypePTR) {
					CFStringRef	name;
					char		str[MAXHOSTNAMELEN];

					name = CFArrayGetValueAtIndex(targetPrivate->resolvedAddresses, i);
					_SC_cfstring_to_cstring(name, str, sizeof(str), kCFStringEncodingUTF8);
					xpc_array_set_string(addresses, XPC_ARRAY_APPEND, str);
				}
			}

			xpc_dictionary_set_value(reply,
						 REACH_STATUS_RESOLVED_ADDRESSES,
						 addresses);
			xpc_release(addresses);
		}
		xpc_dictionary_set_int64(reply,
					 REACH_STATUS_RESOLVED_ERROR,
					 targetPrivate->resolvedError);
		xpc_dictionary_set_uint64(reply,
					  REACH_STATUS_DNS_FLAGS,
					  targetPrivate->dnsFlags);

	}

	MUTEX_UNLOCK(&targetPrivate->lock);

	return;
}


#pragma mark -


typedef struct {
	xpc_connection_t	connection;
	uint64_t		target_id;
} reach_watcher_key_t;

typedef struct {
	unsigned int		n_changes;
} reach_watcher_val_t;


static CFDataRef
_target_watcher_key_create(xpc_connection_t	connection,
			   uint64_t		target_id)
{
	CFDataRef		key;
	reach_watcher_key_t	watcher_key;

	watcher_key.connection = connection;
	watcher_key.target_id  = target_id;

	key = CFDataCreate(NULL, (UInt8 *)&watcher_key, sizeof(watcher_key));
	return key;
}


static Boolean
_target_watcher_add(SCNetworkReachabilityRef	target,
		    xpc_connection_t		connection,
		    uint64_t			target_id)
{
	__block Boolean		ok	= TRUE;
	dispatch_queue_t	q;

	q = _target_queue(target);
	dispatch_sync(q, ^{
		CFDataRef			key;
		SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

		if (targetPrivate->serverWatchers == NULL) {
			ok = SCNetworkReachabilitySetDispatchQueue(target, q);
			if (!ok) {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("<%p> target %p: _watcher_add SCNetworkReachabilitySetDispatchQueue() failed: %s"),
				      connection,
				      target,
				      SCErrorString(SCError()));
				return;
			}

			targetPrivate->serverWatchers = CFDictionaryCreateMutable(NULL,
										  0,
										  &kCFTypeDictionaryKeyCallBacks,
										  &kCFTypeDictionaryValueCallBacks);
		}

		xpc_retain(connection);

		key = _target_watcher_key_create(connection, target_id);
		if (CFDictionaryContainsKey(targetPrivate->serverWatchers, key)) {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("<%p>   target %p: watcher not added, c=0x%0llx, \"serverWatchers\" key exists"),
			      connection,
			      target,
			      target_id);
		} else {
			CFDataRef				val;
			static const reach_watcher_val_t	watcher_val0	= { 0 };

			val = CFDataCreate(NULL, (UInt8 *)&watcher_val0, sizeof(watcher_val0));
			CFDictionaryAddValue(targetPrivate->serverWatchers, key, val);
			CFRelease(val);

			if (S_debug) {
				SCLog(TRUE, LOG_INFO,
				      CFSTR("<%p>   target %p: watcher added, c=0x%0llx, n=%ld"),
				      connection,
				      target,
				      target_id,
				      CFDictionaryGetCount(targetPrivate->serverWatchers));
			}
		}
		CFRelease(key);
	});

	return ok;
}


static Boolean
_target_watcher_checkin(SCNetworkReachabilityRef	target,
			xpc_connection_t		connection,
			uint64_t			target_id)
{
	__block Boolean		scheduled	= FALSE;

	dispatch_sync(_target_queue(target), ^{
		CFDataRef			key;
		unsigned int			n;
		SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;
		CFDataRef			val;
		reach_watcher_val_t		*watcher_val;

		if (targetPrivate->serverWatchers == NULL) {
			// if no watchers
			return;
		}

		key = _target_watcher_key_create(connection, target_id);
		val = CFDictionaryGetValue(targetPrivate->serverWatchers, key);
		CFRelease(key);
		if (val == NULL) {
			// if the target [for this client] was not scheduled
			return;
		}

		// indicate that the target was scheduled
		scheduled = TRUE;

		/*
		 * and note that the reachability flags for this target have
		 * been picked up by the client
		 */
		/* ALIGN: CF aligns to at least >8 byte boundries */
		watcher_val = (reach_watcher_val_t *)(void *)CFDataGetBytePtr(val);
		n = _SC_ATOMIC_ZERO(&watcher_val->n_changes);
		if (S_debug && (n > 0)) {
			SCLog(TRUE, LOG_INFO,
			      CFSTR("<%p> target %p: SCNetworkReachabilityGetFlags() after %d notification%s"),
			      connection,
			      target,
			      n,
			      (n == 1) ? "" : "s");
		}
	});

	return scheduled;
}


static Boolean
_target_watcher_remove(SCNetworkReachabilityRef	target,
		       xpc_connection_t		connection,
		       uint64_t			target_id)
{
	__block Boolean		ok	= TRUE;

	dispatch_sync(_target_queue(target), ^{
		CFDataRef			key;
		CFIndex				n;
		SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

		if (targetPrivate->serverWatchers == NULL) {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("<%p>   target %p: watcher not removed, c=0x%0llx, no \"serverWatchers\""),
			      connection,
			      target,
			      target_id);
			ok = FALSE;
			return;
		}

		key = _target_watcher_key_create(connection, target_id);
		if (!CFDictionaryContainsKey(targetPrivate->serverWatchers, key)) {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("<%p>   target %p: watcher not removed, c=0x%0llx, no \"serverWatchers\" key"),
			      connection,
			      target,
			      target_id);
			CFRelease(key);
			ok = FALSE;
			return;
		}

		CFDictionaryRemoveValue(targetPrivate->serverWatchers, key);
		CFRelease(key);

		n = CFDictionaryGetCount(targetPrivate->serverWatchers);

		if (S_debug) {
			SCLog(TRUE, LOG_INFO,
			      CFSTR("<%p>   target %p: watcher removed, c=0x%0llx, n=%ld"),
			      connection,
			      target,		// server
			      target_id,	// client
			      n);
		}

		if (n == 0) {
			CFRelease(targetPrivate->serverWatchers);
			targetPrivate->serverWatchers = NULL;

			ok = SCNetworkReachabilitySetDispatchQueue(target, NULL);
			if (!ok) {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("<%p> target %p: _watcher_remove SCNetworkReachabilitySetDispatchQueue() failed: %s"),
				      connection,
				      target,
				      SCErrorString(SCError()));
				return;
			}

			// no more watchers, flags are no longer valid
			if (_SC_ATOMIC_CMPXCHG(&targetPrivate->serverInfoValid, TRUE, FALSE)) {
				if (S_debug) {
					SCLog(TRUE, LOG_INFO, CFSTR("%s  flags are no longer \"valid\""),
					      targetPrivate->log_prefix);
				}
			}
		}

		xpc_release(connection);
	});

	return ok;
}


#pragma mark -
#pragma mark Reachability [RBT] client support


typedef struct {
	rb_node_t		rbn;
	xpc_connection_t	connection;
	pid_t			pid;
	const char		*proc_name;
	CFMutableDictionaryRef	targets;	// target_id --> SCNetworkReachabilityRef
} reach_client_t;


static int
_rbt_compare_transaction_nodes(void *context, const void *n1, const void *n2)
{
	uint64_t	a = (uintptr_t)((reach_client_t *)n1)->connection;
	uint64_t	b = (uintptr_t)((reach_client_t *)n2)->connection;

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
	uint64_t	a = (uintptr_t)((reach_client_t *)n1)->connection;
	uint64_t	b = *(uintptr_t *)key;

	if (a == b) {
		return 0;
	} else if (a < b) {
		return -1;
	} else {
		return 1;
	}
}


static rb_tree_t *
_reach_clients_rbt()
{
	static dispatch_once_t		once;
	static const rb_tree_ops_t	ops = {
		.rbto_compare_nodes	= _rbt_compare_transaction_nodes,
		.rbto_compare_key	= _rbt_compare_transaction_key,
		.rbto_node_offset	= offsetof(reach_client_t, rbn),
		.rbto_context		= NULL
	};
	static rb_tree_t		rbt;

	dispatch_once(&once, ^{
		rb_tree_init(&rbt, &ops);
	});

	return &rbt;
}


static dispatch_queue_t
_reach_clients_rbt_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(REACH_SERVICE_NAME ".clients.rbt", NULL);
	});

	return q;
}


static reach_client_t *
_reach_client_create(xpc_connection_t connection)
{
	reach_client_t	*client;

	client = calloc(1, sizeof(*client));
	client->connection = connection;
	client->pid = xpc_connection_get_pid(connection);
	client->proc_name = NULL;
	client->targets = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);

	return client;
}


static void
_reach_client_release(reach_client_t *client)
{
	if (client->proc_name != NULL) {
		free((void *)client->proc_name);
	}
	CFRelease(client->targets);
	free(client);
	return;
}


static void
_reach_client_remove_target(const void *key, const void *value, void *context)
{
	xpc_connection_t		connection	= (xpc_connection_t)context;
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)value;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	// check if we have anyone watching this target
	if (targetPrivate->serverWatchers != NULL) {
		CFIndex		n;

		n = CFDictionaryGetCount(targetPrivate->serverWatchers);
		if (n > 0) {
			CFIndex		i;
			CFDictionaryRef	serverWatchers;
			const void *	watchers_q[32];
			const void **	watchers	= watchers_q;

			serverWatchers = CFDictionaryCreateCopy(NULL, targetPrivate->serverWatchers);

			if (n > sizeof(watchers_q)/sizeof(watchers[0])) {
				watchers = CFAllocatorAllocate(NULL, n * sizeof(CFDataRef), 0);
			}
			CFDictionaryGetKeysAndValues(serverWatchers, watchers, NULL);

			for (i = 0; i < n; i++) {
				CFDataRef		key;
				reach_watcher_key_t	*watcher_key;

				key = (CFDataRef)watchers[i];
				/* ALIGN: CF aligns to >8 byte boundries */
				watcher_key = (reach_watcher_key_t *)(void *)CFDataGetBytePtr(key);
				if (watcher_key->connection == connection) {
					// remove watcher references for THIS connection
					_target_watcher_remove(target,
							       watcher_key->connection,
							       watcher_key->target_id);
				}
			}

			if (watchers != watchers_q) {
				CFAllocatorDeallocate(NULL, watchers);
			}

			CFRelease(serverWatchers);
		}
	}

	// remove our reference to this target
	dispatch_sync(_server_digest_queue(), ^{
		_target_reference_remove(target, connection);
	});

	return;
}


static void
_reach_client_remove(xpc_connection_t connection)
{
	uint64_t	connection_id	= (uintptr_t)connection;

	dispatch_sync(_reach_clients_rbt_queue(), ^{
		reach_client_t	*client;
		rb_tree_t	*rbt	= _reach_clients_rbt();

		client = rb_tree_find_node(rbt, &connection_id);
		if (client != NULL) {
			// remove any remaining target references (for this client)
			my_CFDictionaryApplyFunction(client->targets,
						     _reach_client_remove_target,
						     (void *)connection);

			rb_tree_remove_node(rbt, client);
			_reach_client_release(client);
		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("<%p> _reach_client_remove: unexpected client"),
			      connection);
		}
	});

	return;
}


static __inline__ CFDataRef
_client_target_key_create(uint64_t target_id)
{
	CFDataRef	target_key;

	target_key = CFDataCreate(NULL, (UInt8 *)&target_id, sizeof(target_id));
	return target_key;
}


static SCNetworkReachabilityRef
_client_target_copy(reach_client_t *client, uint64_t target_id)
{
	SCNetworkReachabilityRef	target;
	CFDataRef			target_key;

	target_key = _client_target_key_create(target_id);
	target = CFDictionaryGetValue(client->targets, target_key);
	CFRelease(target_key);

	if (target != NULL) {
		CFRetain(target);
	}

	return target;
}


static Boolean
_client_target_set(reach_client_t *client, uint64_t target_id, SCNetworkReachabilityRef target)
{
	Boolean		added;
	CFDataRef	target_key;

	target_key = _client_target_key_create(target_id);
	added = !CFDictionaryContainsKey(client->targets, target_key);
	if (added) {
		CFDictionarySetValue(client->targets, target_key, target);
	}
	CFRelease(target_key);

	return added;
}


static void
_client_target_remove(reach_client_t *client, uint64_t target_id)
{
	CFDataRef	target_key;

	target_key = _client_target_key_create(target_id);
	CFDictionaryRemoveValue(client->targets, target_key);
	CFRelease(target_key);

	return;
}


#pragma mark -
#pragma mark Reachability [XPC] server functions


static dispatch_queue_t
_reach_server_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(REACH_SERVICE_NAME, NULL);
	});

	return q;
}


/*
 * _reach_changed
 *
 * Note: should be exec'd on the target queue
 */
static void
_reach_changed(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void *info)
{
	CFIndex				i;
	CFIndex				n;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;
	const void *			watcher_keys_q[32];
	const void **			watcher_keys	= watcher_keys_q;
	const void *			watcher_vals_q[32];
	const void **			watcher_vals	= watcher_vals_q;

	if (S_debug) {
		SCLog(TRUE, LOG_INFO,
		      CFSTR("%sprocess reachability changed, flags = 0x%08x"),
		      targetPrivate->log_prefix,
		      flags);
	}

	if (targetPrivate->serverWatchers == NULL) {
		// if no watchers
		return;
	}

	n = CFDictionaryGetCount(targetPrivate->serverWatchers);
	if (n == 0) {
		// if no watchers
		return;
	}

	if (!isReachabilityTypeName(targetPrivate->type) || !targetPrivate->dnsNoAddressesSinceLastTimeout) {
		/*
		 * Because we are actively watching for additional changes
		 * we mark the flags as "valid"
		 */
		if (_SC_ATOMIC_CMPXCHG(&targetPrivate->serverInfoValid, FALSE, TRUE)) {
			if (S_debug) {
				SCLog(TRUE, LOG_INFO, CFSTR("%s  flags are now \"valid\""),
				      targetPrivate->log_prefix);
			}
		}
	}

	// notify all of the watchers
	if (n > sizeof(watcher_keys_q)/sizeof(watcher_keys[0])) {
		watcher_keys = CFAllocatorAllocate(NULL, n * sizeof(CFDataRef), 0);
		watcher_vals = CFAllocatorAllocate(NULL, n * sizeof(CFDataRef), 0);
	}

	CFDictionaryGetKeysAndValues(targetPrivate->serverWatchers,
				     watcher_keys,
				     watcher_vals);

	for (i = 0; i < n; i++) {
		xpc_connection_t	connection;
		CFDataRef		key;
		uint64_t		target_id;
		CFDataRef		val;
		reach_watcher_key_t	*watcher_key;
		reach_watcher_val_t	*watcher_val;

		val = (CFDataRef)watcher_vals[i];
		/* ALIGN: CF aligns to >8 byte boundries */
		watcher_val = (reach_watcher_val_t *)(void *)CFDataGetBytePtr(val);

		if (_SC_ATOMIC_INC(&watcher_val->n_changes) > 0) {
			// if we've already sent a notification
			continue;
		}

		key = (CFDataRef)watcher_keys[i];
		/* ALIGN: CF aligns to >8 byte boundries */
		watcher_key = (reach_watcher_key_t *)(void *)CFDataGetBytePtr(key);

		connection = xpc_retain(watcher_key->connection);
		target_id  = watcher_key->target_id;
		dispatch_async(_reach_server_queue(), ^{
			xpc_object_t	reply;

			// create our [async] notification
			reply = xpc_dictionary_create(NULL, NULL, 0);

			// set notification
			xpc_dictionary_set_int64(reply,
						 MESSAGE_NOTIFY,
						 MESSAGE_REACHABILITY_STATUS);

			// set target ID
			xpc_dictionary_set_uint64(reply,
						  REACH_CLIENT_TARGET_ID,
						  target_id);

			log_xpc_object("  reply [async]", reply);
			xpc_connection_send_message(connection, reply);

			xpc_release(reply);
			xpc_release(connection);
		});
	}

	if (n > sizeof(watcher_keys_q)/sizeof(watcher_keys[0])) {
		CFAllocatorDeallocate(NULL, watcher_keys);
		CFAllocatorDeallocate(NULL, watcher_vals);
	}

	return;
}


static void
sanitize_address(const struct sockaddr *from, struct sockaddr *to)
{
	switch (from->sa_family) {
		case AF_INET : {
			/* ALIGN: cast okay, alignment not assumed. */
			struct sockaddr_in *from4	= (struct sockaddr_in *)(void *)from;
			struct sockaddr_in *to4		= (struct sockaddr_in *)(void *)to;

			bzero(to4, sizeof(*to4));
			to4->sin_len = sizeof(*to4);
			to4->sin_family = AF_INET;
			bcopy(&from4->sin_addr, &to4->sin_addr, sizeof(to4->sin_addr));
			break;
		}

		case AF_INET6 : {
			/* ALIGN: cast okay, alignment not assumed. */
			struct sockaddr_in6 *from6	= (struct sockaddr_in6 *)(void *)from;
			struct sockaddr_in6 *to6	= (struct sockaddr_in6 *)(void *)to;

			bzero(to6, sizeof(*to6));
			to6->sin6_len = sizeof(*to6);
			to6->sin6_family = AF_INET6;
			bcopy(&from6->sin6_addr, &to6->sin6_addr, sizeof(to6->sin6_addr));
			to6->sin6_scope_id = from6->sin6_scope_id;
			break;
		}

		default:
			bcopy(from, to, from->sa_len);
			break;
	}

	return;
}


static void
target_add(reach_client_t *client, xpc_object_t request)
{
	const char				*name;
	const struct sockaddr			*localAddress;
	struct sockaddr_storage			localAddress0;
	const struct sockaddr			*remoteAddress;
	struct sockaddr_storage			remoteAddress0;
	const struct sockaddr			*ptrAddress;
	struct sockaddr_storage			ptrAddress0;
	int64_t					if_index;
	const char				*if_name	= NULL;
	bool					onDemandBypass	= FALSE;
	bool					resolverBypass	= FALSE;
	uint64_t				target_id;
	uid_t					uid		= 0;


	unsigned char				bytes[CC_SHA1_DIGEST_LENGTH];
	CC_SHA1_CTX				ctx;
	CFDataRef				digest		= NULL;
	size_t					len;
	xpc_connection_t			remote;
	xpc_object_t				reply;
	uint64_t				status		= REACH_REQUEST_REPLY_FAILED;

	Boolean					added;
	__block Boolean				ok		= TRUE;
	__block SCNetworkReachabilityRef	target		= NULL;

	if (S_debug) {
		SCLog(TRUE, LOG_INFO,
		      CFSTR("<%p> create reachability target"),
		      client->connection);
//		log_xpc_object("  create", request);
	}

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);
	if (reply == NULL) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("<%p> target_add: xpc_dictionary_create_reply: failed"),
		      client->connection);
		return;
	}

	target_id = xpc_dictionary_get_uint64(request, REACH_CLIENT_TARGET_ID);
	if (target_id == 0) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "no target ID");
		goto done;
	}

	// create a "digest" of the [new] target

	CC_SHA1_Init(&ctx);

	name = xpc_dictionary_get_string(request, REACH_TARGET_NAME);
	if (name != NULL) {
		CC_SHA1_Update(&ctx, REACH_TARGET_NAME, sizeof(REACH_TARGET_NAME));
		CC_SHA1_Update(&ctx, name, (CC_LONG)strlen(name));
	}

	localAddress = xpc_dictionary_get_data(request, REACH_TARGET_LOCAL_ADDR, &len);
	if (localAddress != NULL) {
		if ((len == localAddress->sa_len) && (len <= sizeof(struct sockaddr_storage))) {
			sanitize_address(localAddress, (struct sockaddr *)&localAddress0);
			CC_SHA1_Update(&ctx, REACH_TARGET_LOCAL_ADDR, sizeof(REACH_TARGET_LOCAL_ADDR));
			CC_SHA1_Update(&ctx, &localAddress0, (CC_LONG)len);
		} else {
			xpc_dictionary_set_string(reply,
						  REACH_REQUEST_REPLY_DETAIL,
						  "local address: size error");
			goto done;
		}
	}

	remoteAddress = xpc_dictionary_get_data(request, REACH_TARGET_REMOTE_ADDR, &len);
	if (remoteAddress != NULL) {
		if ((len == remoteAddress->sa_len) && (len <= sizeof(struct sockaddr_storage))) {
			sanitize_address(remoteAddress, (struct sockaddr *)&remoteAddress0);
			CC_SHA1_Update(&ctx, REACH_TARGET_REMOTE_ADDR, sizeof(REACH_TARGET_REMOTE_ADDR));
			CC_SHA1_Update(&ctx, &remoteAddress0, (CC_LONG)len);
		} else {
			xpc_dictionary_set_string(reply,
						  REACH_REQUEST_REPLY_DETAIL,
						  "remote address: size error");
			goto done;
		}
	}

	ptrAddress = xpc_dictionary_get_data(request, REACH_TARGET_PTR_ADDR, &len);
	if (ptrAddress != NULL) {
		if ((len == ptrAddress->sa_len) && (len <= sizeof(struct sockaddr_storage))) {
			sanitize_address(ptrAddress, (struct sockaddr *)&ptrAddress0);
			CC_SHA1_Update(&ctx, REACH_TARGET_PTR_ADDR, sizeof(REACH_TARGET_PTR_ADDR));
			CC_SHA1_Update(&ctx, &ptrAddress0, (CC_LONG)len);
		} else {
			xpc_dictionary_set_string(reply,
						  REACH_REQUEST_REPLY_DETAIL,
						  "ptr address: size error");
			goto done;
		}
	}

	if_index = xpc_dictionary_get_int64(request, REACH_TARGET_IF_INDEX);
	if (if_index != 0) {
		if_name = xpc_dictionary_get_string(request, REACH_TARGET_IF_NAME);
		if (if_name != NULL) {
			CC_SHA1_Update(&ctx, REACH_TARGET_IF_NAME, sizeof(REACH_TARGET_IF_NAME));
			CC_SHA1_Update(&ctx, if_name, (CC_LONG)strlen(if_name));
		}
	}


	onDemandBypass = xpc_dictionary_get_bool(request, REACH_TARGET_ONDEMAND_BYPASS);
	if (onDemandBypass) {
		CC_SHA1_Update(&ctx, REACH_TARGET_ONDEMAND_BYPASS, sizeof(REACH_TARGET_ONDEMAND_BYPASS));
		CC_SHA1_Update(&ctx, &onDemandBypass, sizeof(onDemandBypass));
	}

	resolverBypass = xpc_dictionary_get_bool(request, REACH_TARGET_RESOLVER_BYPASS);
	if (resolverBypass) {
		CC_SHA1_Update(&ctx, REACH_TARGET_RESOLVER_BYPASS, sizeof(REACH_TARGET_RESOLVER_BYPASS));
		CC_SHA1_Update(&ctx, &resolverBypass, sizeof(resolverBypass));
	}

	// Grab UID from XPC connection
	uid = xpc_connection_get_euid(remote);
	CC_SHA1_Update(&ctx, &uid, sizeof(uid));


	CC_SHA1_Final(bytes, &ctx);
	digest = CFDataCreate(NULL, bytes, sizeof(bytes));

	/*
	 * Check to see if we already have a SCNetworkReachability object
	 * for this digest. If so, we'll share the existing target. If not,
	 * create a new [shared] target.
	 */
	dispatch_sync(_server_digest_queue(), ^{
		target = CFDictionaryGetValue(reach_digest_map, digest);
		if (target != NULL) {
			CFRetain(target);
		} else {
			CFDataRef			data;
			CFMutableDictionaryRef		options;
			CFStringRef			str;

			options = CFDictionaryCreateMutable(NULL,
							    0,
							    &kCFTypeDictionaryKeyCallBacks,
							    &kCFTypeDictionaryValueCallBacks);
			if (name != NULL) {
				str = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionNodeName, str);
				CFRelease(str);
			}
			if (localAddress != NULL) {
				data = CFDataCreate(NULL, (const UInt8 *)&localAddress0, localAddress0.ss_len);
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionLocalAddress, data);
				CFRelease(data);
			}
			if (remoteAddress != NULL) {
				data = CFDataCreate(NULL, (const UInt8 *)&remoteAddress0, remoteAddress0.ss_len);
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionRemoteAddress, data);
				CFRelease(data);
			}
			if (ptrAddress != NULL) {
				data = CFDataCreate(NULL, (const UInt8 *)&ptrAddress0, ptrAddress0.ss_len);
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionPTRAddress, data);
				CFRelease(data);
			}
			if (onDemandBypass) {
				CFDictionarySetValue(options,
						     kSCNetworkReachabilityOptionConnectionOnDemandBypass,
						     kCFBooleanTrue);
			}
			if (resolverBypass) {
				CFDictionarySetValue(options,
						     kSCNetworkReachabilityOptionResolverBypass,
						     kCFBooleanTrue);
			}
			CFDictionarySetValue(options,
					     kSCNetworkReachabilityOptionServerBypass,
					     kCFBooleanTrue);
			target = SCNetworkReachabilityCreateWithOptions(NULL, options);
			CFRelease(options);
			if (target == NULL) {
				xpc_dictionary_set_string(reply,
							  REACH_REQUEST_REPLY_DETAIL,
							  "SCNetworkReachabilityCreateWithOptions failed");
				ok = FALSE;
				return;
			}

			// Set the UID on the target
			((SCNetworkReachabilityPrivateRef)target)->uid = uid;

			// because the interface name may not (no longer) be valid we set
			// this after we've created the SCNetworkReachability object
			if ((if_index != 0) && (if_name != NULL)) {
				SCNetworkReachabilityPrivateRef	targetPrivate;

				targetPrivate = (SCNetworkReachabilityPrivateRef)target;
				targetPrivate->if_index = (unsigned int)if_index;
				strlcpy(targetPrivate->if_name, if_name, sizeof(targetPrivate->if_name));
			}


			ok = SCNetworkReachabilitySetCallback(target, _reach_changed, NULL);
			if (!ok) {
				xpc_dictionary_set_string(reply,
							  REACH_REQUEST_REPLY_DETAIL,
							  "SCNetworkReachabilitySetCallback failed");
				CFRelease(target);
				target = NULL;
				return;
			}
		}

		// bump the number of references to this target
		_target_reference_add(target, digest, client->connection);
	});

	if (!ok) {
		goto done;
	}

	/*
	 * add an association for the client's target_id to the [shared]
	 * SCNetworkReachability object.
	 */
	added = _client_target_set(client, target_id, target);
	if (!added) {
		// if we already had a reference to the target (e.g. reconnect)
		dispatch_sync(_server_digest_queue(), ^{
			_target_reference_remove(target, client->connection);
		});
	}

	status = REACH_REQUEST_REPLY_OK;

    done :

	xpc_dictionary_set_int64(reply, REACH_REQUEST_REPLY, status);
//	log_xpc_object("  reply", reply);
	xpc_connection_send_message(remote, reply);
	xpc_release(reply);

	if (digest != NULL) CFRelease(digest);
	if (target != NULL) CFRelease(target);
	return;
}


static void
target_remove(reach_client_t *client, xpc_object_t request)
{
	xpc_connection_t		remote;
	xpc_object_t			reply;
	uint64_t			status		= REACH_REQUEST_REPLY_FAILED;
	SCNetworkReachabilityRef	target		= NULL;
	uint64_t			target_id;

	if (S_debug) {
		SCLog(TRUE, LOG_INFO,
		      CFSTR("<%p> remove reachability target"),
		      client->connection);
//		log_xpc_object("  remove", request);
	}

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);
	if (reply == NULL) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("<%p> target_remove: xpc_dictionary_create_reply: failed"),
		      client->connection);
		return;
	}

	target_id = xpc_dictionary_get_uint64(request, REACH_CLIENT_TARGET_ID);
	if (target_id == 0) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "no target ID");
		goto done;
	}

	target = _client_target_copy(client, target_id);
	if (target == NULL) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "no target");
		status = REACH_REQUEST_REPLY_UNKNOWN;
		goto done;
	}

	/*
	 * remove the association from the client's target_id to the [shared]
	 * SCNetworkReachability object.
	 */
	_client_target_remove(client, target_id);

	// drop the number of references to this target
	dispatch_sync(_server_digest_queue(), ^{
		_target_reference_remove(target, client->connection);
	});

	status = REACH_REQUEST_REPLY_OK;

    done :

	xpc_dictionary_set_int64(reply, REACH_REQUEST_REPLY, status);
//	log_xpc_object("  reply", reply);
	xpc_connection_send_message(remote, reply);
	xpc_release(reply);

	if (target != NULL) CFRelease(target);
	return;
}


static void
target_schedule(reach_client_t *client, xpc_object_t request)
{
	Boolean				ok;
	xpc_connection_t		remote;
	xpc_object_t			reply;
	uint64_t			status		= REACH_REQUEST_REPLY_FAILED;
	SCNetworkReachabilityRef	target		= NULL;
	uint64_t			target_id;

	if (S_debug) {
		SCLog(TRUE, LOG_INFO,
		      CFSTR("<%p> schedule reachability target"),
		      client->connection);
//		log_xpc_object("  schedule", request);
	}

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);
	if (reply == NULL) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("<%p> target_schedule: xpc_dictionary_create_reply: failed"),
		      client->connection);
		return;
	}

	target_id = xpc_dictionary_get_uint64(request, REACH_CLIENT_TARGET_ID);
	if (target_id == 0) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "no target ID");
		goto done;
	}

	target = _client_target_copy(client, target_id);
	if (target == NULL) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "no target");
		status = REACH_REQUEST_REPLY_UNKNOWN;
		goto done;
	}

	// enable monitoring
	ok = _target_watcher_add(target, client->connection, target_id);
	if (!ok) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "could not add watcher");
		goto done;
	}

	status = REACH_REQUEST_REPLY_OK;

    done :

	xpc_dictionary_set_int64(reply, REACH_REQUEST_REPLY, status);
//	log_xpc_object("  reply", reply);
	xpc_connection_send_message(remote, reply);
	xpc_release(reply);

	if (target != NULL) CFRelease(target);
	return;
}


static void
target_status(reach_client_t *client, xpc_object_t request)
{
	xpc_connection_t		remote;
	xpc_object_t			reply;
	__block Boolean			reply_now	= TRUE;
	Boolean				scheduled;
	uint64_t			status		= REACH_REQUEST_REPLY_FAILED;
	SCNetworkReachabilityRef	target		= NULL;
	uint64_t			target_id;

	if(S_debug) {
		SCLog(TRUE, LOG_INFO,
		      CFSTR("<%p> get status of reachability target"),
		      client->connection);
//		log_xpc_object("  status", request);
	}

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);
	if (reply == NULL) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("<%p> target_status: xpc_dictionary_create_reply: failed"),
		      client->connection);
		return;
	}

	target_id = xpc_dictionary_get_uint64(request, REACH_CLIENT_TARGET_ID);
	if (target_id == 0) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("<%p>   target_status: no target"),
		      client->connection);
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "no target ID");
		goto done;
	}

	target = _client_target_copy(client, target_id);
	if (target == NULL) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("<%p>   target_status: no target (0x%0llx)"),
		      client->connection,
		      target_id);
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "no target");
		status = REACH_REQUEST_REPLY_UNKNOWN;
		goto done;
	}

	/*
	 * Check to see if the target [for this client] had been "scheduled".
	 *
	 * If so, also mark that we've picked up the current reachability
	 * flags and that any pending notifications have been processed.
	 */
	scheduled = _target_watcher_checkin(target, client->connection, target_id);

	/*
	 * return current reachability information to the caller
	 */
	dispatch_sync(_target_queue(target), ^{
		SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

		if (scheduled) {
			/*
			 * The client "scheduled" this target.  As such, we
			 * know that this is an async query and that we only
			 * need to return the "last known" flags.
			 */
			_target_reply_add_reachability(target, reply);
//			log_xpc_object("  reply [scheduled]", reply);

			/*
			 * ... and if it's not a "name" query then we can mark the
			 * flags as valid.
			 */
			if (!isReachabilityTypeName(targetPrivate->type)) {
				if (_SC_ATOMIC_CMPXCHG(&targetPrivate->serverInfoValid, FALSE, TRUE)) {
					if (S_debug) {
						SCLog(TRUE, LOG_INFO, CFSTR("%s  flags are now \"valid\"."),
						      targetPrivate->log_prefix);
					}
				}
			}

			if (S_debug) {
				CFStringRef	str;

				str = _SCNetworkReachabilityCopyTargetFlags(target);
				SCLog(TRUE, LOG_INFO,
				      CFSTR("<%p>   reply [scheduled%s], %@"),
				      client->connection,
				      targetPrivate->serverInfoValid ? "/valid" : "",
				      str);
				CFRelease(str);
			}
		} else {
			/*
			 * The client has NOT "scheduled" this target.  As
			 * such, we know that this is a sync query and that
			 * we must return "current" flags.
			 */
			if (targetPrivate->scheduled && targetPrivate->serverInfoValid) {
				/*
				 * The server target has been "scheduled" and we
				 * have flags that are "current".
				 */
				_target_reply_add_reachability(target, reply);
//				log_xpc_object("  reply [scheduled/valid]", reply);

				if (S_debug) {
					CFStringRef	str;

					str = _SCNetworkReachabilityCopyTargetFlags(target);
					SCLog(TRUE, LOG_INFO,
					      CFSTR("<%p>   reply [scheduled/valid], %@"),
					      client->connection,
					      str);
					CFRelease(str);
				}
			} else {
				dispatch_group_t	group;

				/*
				 * The server target has NOT been "scheduled" or
				 * we do not have "current" flags.  This means that
				 * we must query for the current information and
				 * return the flags to the client when they are
				 * available.
				 */

				reply_now = FALSE;

				group = _target_group(target);
				if (_SC_ATOMIC_INC(&targetPrivate->serverSyncQueryActive) == 0) {
					CFRetain(target);
					dispatch_group_async(group, _server_concurrent_queue(), ^{
						SCNetworkReachabilityFlags	flags;
						unsigned int			n;
						Boolean				ok;

						// query for the flags
						ok = SCNetworkReachabilityGetFlags(target, &flags);
						flags = targetPrivate->info.flags;	// get the "raw" flags
						if (!ok) {
							SCLog(TRUE, LOG_ERR,
							      CFSTR("SCNetworkReachabilityGetFlags() [sync query] failed"
								    "\n  target = %@"
								    "\n  status = %s"),
							      target,
							      SCErrorString(SCError()));
						}

						/*
						 * if we have current flags, if the target has since been
						 * scheduled, and this is not a "name" query, then mark as
						 * valid.
						 */
						if (ok &&
						    targetPrivate->scheduled &&
						    !isReachabilityTypeName(targetPrivate->type)) {
							if (_SC_ATOMIC_CMPXCHG(&targetPrivate->serverInfoValid, FALSE, TRUE)) {
								if (S_debug) {
									SCLog(TRUE, LOG_INFO, CFSTR("%s  flags are now \"valid\"!"),
									      targetPrivate->log_prefix);
								}
							}
						}

						// sync query complete
						n = _SC_ATOMIC_ZERO(&targetPrivate->serverSyncQueryActive);
						if (S_debug) {
							SCLog(TRUE, LOG_INFO,
							      CFSTR("%sSCNetworkReachabilityGetFlags() [sync query] complete, n = %d"),
							      targetPrivate->log_prefix,
							      n);
						}

						CFRelease(target);
					});
				}

				CFRetain(target);
				dispatch_group_notify(group, _target_queue(target), ^{
					// flags are now available
					_target_reply_add_reachability(target, reply);
					xpc_dictionary_set_int64(reply, REACH_REQUEST_REPLY, REACH_REQUEST_REPLY_OK);
//					log_xpc_object("  reply [delayed]", reply);

					if (S_debug) {
						CFStringRef	str;

						str = _SCNetworkReachabilityCopyTargetFlags(target);
						SCLog(TRUE, LOG_INFO,
						      CFSTR("<%p>   reply [delayed], %@"),
						      client->connection,
						      str);
						CFRelease(str);
					}

					xpc_connection_send_message(remote, reply);
					xpc_release(reply);

					CFRelease(target);
				});
			}
		}
	});

	status = REACH_REQUEST_REPLY_OK;

    done :

	if (reply_now) {
		xpc_dictionary_set_int64(reply, REACH_REQUEST_REPLY, status);

		if (status != REACH_REQUEST_REPLY_OK) {
//			log_xpc_object("  reply [!]", reply);

			if (S_debug) {
				SCLog(TRUE, LOG_INFO,
				      CFSTR("<%p>   reply [!]"),
				      client->connection);
			}
		}

		xpc_connection_send_message(remote, reply);
		xpc_release(reply);
	} else if (S_debug) {
		CFStringRef	str;

		str = _SCNetworkReachabilityCopyTargetFlags(target);
		SCLog(TRUE, LOG_INFO,
		      CFSTR("<%p>   no reply [yet], %@"),
		      client->connection,
		      str);
		CFRelease(str);
	}

	if (target != NULL) CFRelease(target);
	return;
}


static void
target_unschedule(reach_client_t *client, xpc_object_t request)
{
	xpc_connection_t		remote;
	xpc_object_t			reply;
	uint64_t			status		= REACH_REQUEST_REPLY_FAILED;
	SCNetworkReachabilityRef	target		= NULL;
	uint64_t			target_id;

	if (S_debug) {
		SCLog(TRUE, LOG_INFO,
		      CFSTR("<%p> unschedule reachability target"),
		      client->connection);
//		log_xpc_object("  unschedule", request);
	}

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);
	if (reply == NULL) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("<%p> target_unschedule: xpc_dictionary_create_reply: failed"),
		      client->connection);
		return;
	}

	target_id = xpc_dictionary_get_uint64(request, REACH_CLIENT_TARGET_ID);
	if (target_id == 0) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "no target ID");
		goto done;
	}

	target = _client_target_copy(client, target_id);
	if (target == NULL) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "no target");
		status = REACH_REQUEST_REPLY_UNKNOWN;
		goto done;
	}

	// disable monitoring
	_target_watcher_remove(target, client->connection, target_id);

	status = REACH_REQUEST_REPLY_OK;

    done :

	xpc_dictionary_set_int64(reply, REACH_REQUEST_REPLY, status);
//	log_xpc_object("  reply", reply);
	xpc_connection_send_message(remote, reply);
	xpc_release(reply);

	if (target != NULL) CFRelease(target);
	return;
}


#define	SNAPSHOT_PATH_STATE	_PATH_VARTMP "configd-reachability"


static void
_snapshot_digest_watcher(const void *key, const void *value, void *context)
{
	__block reach_client_t	*client		= NULL;
	FILE			*f		= (FILE *)context;
	static reach_client_t	no_client	= {
		.pid = 0,
		.proc_name = "?",
	};
	reach_watcher_key_t	*watcher_key;
	reach_watcher_val_t	*watcher_val;

	/* ALIGN: CF aligns to >8 byte boundries */
	watcher_key = (reach_watcher_key_t *)(void *)CFDataGetBytePtr(key);
	watcher_val = (reach_watcher_val_t *)(void *)CFDataGetBytePtr(value);

	dispatch_sync(_reach_clients_rbt_queue(), ^{
		uint64_t	connection_id	= (uintptr_t)watcher_key->connection;
		rb_tree_t	*rbt		= _reach_clients_rbt();

		client = rb_tree_find_node(rbt, &connection_id);
		if (client == NULL) {
			client = &no_client;
		}
	});

	SCPrint(TRUE, f,
		CFSTR("      connection = %p, target(c) = 0x%0llx, command = %s, pid = %d, changes = %u\n"),
		watcher_key->connection,
		watcher_key->target_id,
		client->proc_name,
		client->pid,
		watcher_val->n_changes);

	return;
}


static void
_snapshot_digest(const void *key, const void *value, void *context)
{
	FILE				*f		= (FILE *)context;
	CFStringRef			digest		= (CFStringRef)key;
	dispatch_queue_t		q;
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)value;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	q = _target_queue(target);
	dispatch_sync(q, ^{
		CFIndex		nWatchers	= 0;

		if (targetPrivate->serverWatchers != NULL) {
			nWatchers = CFDictionaryGetCount(targetPrivate->serverWatchers);
		}

		SCPrint(TRUE, f, CFSTR("\n  digest : %@\n"), digest);
		SCPrint(TRUE, f, CFSTR("    %@\n"), target);
		SCPrint(TRUE, f, CFSTR("    valid = %s, async watchers = %ld, sync queries = %u, refs = %u\n"),
			targetPrivate->serverInfoValid ? "Y" : "N",
			nWatchers,
			targetPrivate->serverSyncQueryActive,
			targetPrivate->serverReferences);

		SCPrint(TRUE, f, CFSTR("    network %ld.%3.3d"),
			targetPrivate->last_network.tv_sec,
			targetPrivate->last_network.tv_usec / 1000);
#if	!TARGET_OS_IPHONE
		SCPrint(TRUE, f, CFSTR(", power %ld.%3.3d"),
			targetPrivate->last_power.tv_sec,
			targetPrivate->last_power.tv_usec / 1000);
#endif	// !TARGET_OS_IPHONE
		if (isReachabilityTypeName(targetPrivate->type)) {
			SCPrint(TRUE, f, CFSTR(", DNS %ld.%3.3d"),
				targetPrivate->last_dns.tv_sec,
				targetPrivate->last_dns.tv_usec / 1000);
			if (timerisset(&targetPrivate->dnsQueryEnd)) {
				struct timeval	dnsQueryElapsed;

				timersub(&targetPrivate->dnsQueryEnd,
					 &targetPrivate->dnsQueryStart,
					 &dnsQueryElapsed);
				SCPrint(TRUE, f, CFSTR(" (query %ld.%3.3d / reply %ld.%3.3d)"),
					targetPrivate->dnsQueryStart.tv_sec,
					targetPrivate->dnsQueryStart.tv_usec / 1000,
					dnsQueryElapsed.tv_sec,
					dnsQueryElapsed.tv_usec / 1000);
			}
		}
		if (timerisset(&targetPrivate->last_push)) {
			SCPrint(TRUE, f, CFSTR(", last notify %ld.%3.3d"),
				targetPrivate->last_push.tv_sec,
				targetPrivate->last_push.tv_usec / 1000);
		}
		SCPrint(TRUE, f, CFSTR(", uid %d"),
			targetPrivate->uid);
		SCPrint(TRUE, f, CFSTR("\n"));

		if (nWatchers > 0) {
			CFDictionaryApplyFunction(targetPrivate->serverWatchers,
						  _snapshot_digest_watcher,
						  f);
		}
	});

	return;
}


static void
_snapshot_target(const void *key, const void *value, void *context)
{
	FILE				*f		= (FILE *)context;
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)value;
	uint64_t			target_id;
	CFDataRef			target_key	= (CFDataRef)key;

	/* ALIGN: CF aligns > 8 byte boundries */
	target_id = *(uint64_t *)(void *)CFDataGetBytePtr(target_key);

	SCPrint(TRUE, f,
		CFSTR("    target(c) = 0x%0llx, target(s) = %@\n"),
		target_id,
		target);

	return;
}


static void
_snapshot(reach_client_t *client, xpc_object_t request)
{
	uid_t			euid;
	FILE			*f;
	int			fd;
	Boolean			ok	= FALSE;
	xpc_connection_t	remote;
	xpc_object_t		reply;

	if (S_debug) {
		SCLog(TRUE, LOG_INFO,
		      CFSTR("<%p> snapshot"),
		      client->connection);
//		log_xpc_object("  create", request);
	}

	remote = xpc_dictionary_get_remote_connection(request);
	reply = xpc_dictionary_create_reply(request);
	if (reply == NULL) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("<%p> _snapshot: xpc_dictionary_create_reply: failed"),
		      client->connection);
		return;
	}

	euid = xpc_connection_get_euid(remote);
	if (euid != 0) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "Permission denied.");
		goto done;
	}

	// Save a snapshot of the SCNetworkReachability server "state"

	(void) unlink(SNAPSHOT_PATH_STATE);
	fd = open(SNAPSHOT_PATH_STATE, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0644);
	if (fd == -1) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "open: failed");
		goto done;
	}
	f = fdopen(fd, "w");
	if (f == NULL) {
		xpc_dictionary_set_string(reply,
					  REACH_REQUEST_REPLY_DETAIL,
					  "fdopen: failed");
		goto done;
	}

	// provide connection/client info

	dispatch_sync(_reach_clients_rbt_queue(), ^{
		rb_tree_t	*rbt	= _reach_clients_rbt();

		SCPrint(TRUE, f, CFSTR("Clients :\n"));

		if (rb_tree_count(rbt) > 0) {
			reach_client_t	*client;

			RB_TREE_FOREACH(client, rbt) {
				SCPrint(TRUE, f,
					CFSTR("\n  connection = %p, client = %p, command = %s, pid = %d\n"),
					client->connection,
					client,
					client->proc_name != NULL ? client->proc_name : "?",
					client->pid);
				my_CFDictionaryApplyFunction(client->targets,
							     _snapshot_target,
							     f);
			}
		} else {
			SCPrint(TRUE, f, CFSTR("  None.\n"));
		}

		SCPrint(TRUE, f, CFSTR("\n"));
	});

	// provide "digest" info

	SCPrint(TRUE, f, CFSTR("Digests :\n"));
	dispatch_sync(_server_digest_queue(), ^{
		if (reach_digest_map != NULL) {
			CFDictionaryApplyFunction(reach_digest_map,
						  _snapshot_digest,
						  f);
		}
	});

	(void) fclose(f);

	ok = TRUE;

    done :

	xpc_dictionary_set_int64(reply,
				 REACH_REQUEST_REPLY,
				 ok ? REACH_REQUEST_REPLY_OK : REACH_REQUEST_REPLY_FAILED);
//	log_xpc_object("  reply", reply);
	xpc_connection_send_message(remote, reply);
	xpc_release(reply);

	return;
}


static __inline__ void
_extract_client_info(reach_client_t *client, xpc_object_t request)
{
	// if available/needed, save the process name
	if (client->proc_name == NULL) {
		const char	*proc_name;

		proc_name = xpc_dictionary_get_string(request, REACH_CLIENT_PROC_NAME);
		if (proc_name != NULL) {
			client->proc_name = strdup(proc_name);
		}
	}

	return;
}


static void
process_request(reach_client_t *client, xpc_object_t request)
{
	int64_t		op;

	op = xpc_dictionary_get_int64(request, REACH_REQUEST);
	switch (op) {
		case REACH_REQUEST_CREATE :
			_extract_client_info(client, request);
			target_add(client, request);
			break;
		case REACH_REQUEST_REMOVE :
			target_remove(client, request);
			break;
		case REACH_REQUEST_STATUS :
			target_status(client, request);
			break;
		case REACH_REQUEST_SCHEDULE :
			target_schedule(client, request);
			break;
		case REACH_REQUEST_UNSCHEDULE :
			target_unschedule(client, request);
			break;
		case REACH_REQUEST_SNAPSHOT :
			_extract_client_info(client, request);
			_snapshot(client, request);
			break;
		default :
			SCLog(TRUE, LOG_ERR,
			      CFSTR("<%p> unknown request : %lld"),
			      client->connection,
			      op);
			break;
	}

	return;
}


static void
process_new_connection(xpc_connection_t connection)
{
	reach_client_t	*client;

	if (S_debug) {
		SCLog(TRUE, LOG_INFO, CFSTR("<%p> new reach client, pid=%d"),
		      connection,
		      xpc_connection_get_pid(connection));
	}

	client = _reach_client_create(connection);
	assert(client != NULL);

	dispatch_sync(_reach_clients_rbt_queue(), ^{
		rb_tree_t	*rbt	= _reach_clients_rbt();

		rb_tree_insert_node(rbt, client);
	});

	xpc_connection_set_target_queue(connection, _reach_server_queue());

	xpc_connection_set_event_handler(connection, ^(xpc_object_t xobj) {
		xpc_type_t	type;

		type = xpc_get_type(xobj);
		if (type == XPC_TYPE_DICTIONARY) {
			__block reach_client_t	*client	= NULL;

			dispatch_sync(_reach_clients_rbt_queue(), ^{
				uint64_t	connection_id	= (uintptr_t)connection;
				rb_tree_t	*rbt		= _reach_clients_rbt();

				client = rb_tree_find_node(rbt, &connection_id);
			});

			if (client != NULL) {
				// process the request
				process_request(client, xobj);
			} else {
				char		*desc;

				SCLog(TRUE, LOG_ERR,
				      CFSTR("<%p:%d> unexpected SCNetworkReachability request"),
				      connection,
				      xpc_connection_get_pid(connection));

				desc = xpc_copy_description(xobj);
				SCLog(TRUE, LOG_ERR,
				      CFSTR("  request = %s"),
				      desc);
				free(desc);

				xpc_connection_cancel(connection);
			}

		} else if (type == XPC_TYPE_ERROR) {
			const char	*desc;

			desc = xpc_dictionary_get_string(xobj, XPC_ERROR_KEY_DESCRIPTION);
			if (xobj == XPC_ERROR_CONNECTION_INVALID) {
				if (S_debug) {
					SCLog(TRUE, LOG_INFO,
					      CFSTR("<%p:%d> %s"),
					      connection,
					      xpc_connection_get_pid(connection),
					      desc);
				}

				_reach_client_remove(connection);

			} else if (xobj == XPC_ERROR_CONNECTION_INTERRUPTED) {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("<%p:%d> %s"),
				      connection,
				      xpc_connection_get_pid(connection),
				      desc);

			} else {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("<%p:%d> Connection error: %p : %s"),
				      connection,
				      xpc_connection_get_pid(connection),
				      xobj,
				      desc);
			}

		}  else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("<%p:%d> unknown event type : %p"),
			      connection,
			      xpc_connection_get_pid(connection),
			      type);
		}
	});

	xpc_connection_resume(connection);

	return;
}


#pragma mark -
#pragma mark Reachability server "main"


__private_extern__
void
load_SCNetworkReachability(CFBundleRef bundle, Boolean bundleVerbose)
{
	xpc_connection_t	connection;
	const char		*name;

	S_debug = bundleVerbose;

	/*
	 * create a dictionary mapping SCNetworkReachability [CFData] digests
	 * to SCNetworkReachability objects.
	 */
	reach_digest_map = CFDictionaryCreateMutable(NULL,
						     0,
						     &kCFTypeDictionaryKeyCallBacks,
						     &kCFTypeDictionaryValueCallBacks);

	// create XPC listener
	name = getenv("REACH_SERVER");
	if (name == NULL) {
		name = REACH_SERVICE_NAME;
	}
	connection = xpc_connection_create_mach_service(name,
							_reach_server_queue(),
							XPC_CONNECTION_MACH_SERVICE_LISTENER);

	xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {
		xpc_type_t	type;

		type = xpc_get_type(event);
		if (type == XPC_TYPE_CONNECTION) {
			process_new_connection(event);

		} else if (type == XPC_TYPE_ERROR) {
			const char	*desc;

			desc = xpc_dictionary_get_string(event, XPC_ERROR_KEY_DESCRIPTION);
			if (event == XPC_ERROR_CONNECTION_INVALID) {
				SCLog(TRUE, LOG_ERR, CFSTR("reach server: %s"), desc);
				xpc_release(connection);
			} else if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
				SCLog(TRUE, LOG_ERR, CFSTR("reach server: %s"), desc);
			} else {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("reach server: Connection error: %p : %s"),
				      event,
				      desc);
			}

		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("reach server: unknown event type : %p"),
			      type);
		}
	});
	xpc_connection_resume(connection);

	return;
}

#ifdef  MAIN

int
main(int argc, char **argv)
{
//	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;
	_sc_debug   = TRUE;

	load_SCNetworkReachability(CFBundleGetMainBundle(), (argc > 1) ? TRUE : FALSE);
	CFRunLoopRun();
	/* not reached */
	exit(0);
	return 0;
}

#endif  /* MAIN */
