/*
 * Copyright (c) 2013, 2014 Apple Inc.
 * All rights reserved.
 */
#include <errno.h>
#include <syslog.h>
#include <xpc/xpc.h>

#include "SNHelperPrivate.h"

static xpc_connection_t
create_connection(dispatch_queue_t queue)
{
	xpc_connection_t new_connection;

	new_connection = xpc_connection_create_mach_service(kSNHelperService, queue, XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);
	if (isa_xpc_connection(new_connection)) {
		xpc_connection_set_event_handler(new_connection,
			^(xpc_object_t message) {
				if (isa_xpc_error(message)) {
					syslog(LOG_INFO, "Got an error on the snhelper connection");
				} else if (isa_xpc_dictionary(message)) {
					syslog(LOG_INFO, "Got an unexpected message on the snhelper connection");
				}
			});
		xpc_connection_resume(new_connection);
	}

	return new_connection;
}

static xpc_object_t
copy_response(xpc_object_t request)
{
	dispatch_queue_t conn_queue = dispatch_queue_create("snhelper request", NULL);
	xpc_connection_t connection = create_connection(conn_queue);
	xpc_object_t response = NULL;

	if (connection) {
		response = xpc_connection_send_message_with_reply_sync(connection, request);
		xpc_connection_cancel(connection);
		xpc_release(connection);
	}

	dispatch_release(conn_queue);

	return response;
}

static int
flow_divert_uuid_policy_operate(const uuid_t uuid, int operation)
{
	int result = 0;
	xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_object_t response;

	xpc_dictionary_set_uint64(request, kSNHelperMessageType, operation);
	xpc_dictionary_set_uuid(request, kSNHelperMessageUUID, uuid);

	response = copy_response(request);

	if (isa_xpc_dictionary(response)) {
		result = (int)xpc_dictionary_get_int64(response, kSNHelperMessageResult);
	} else {
		result = EINVAL;
	}

	xpc_release(response);
	xpc_release(request);

	return result;
}

int
snhelper_flow_divert_uuid_policy_add(const uuid_t uuid)
{
	return flow_divert_uuid_policy_operate(uuid, kSNHelperMessageTypeFlowDivertUUIDAdd);
}

int
snhelper_flow_divert_uuid_policy_remove(const uuid_t uuid)
{
	return flow_divert_uuid_policy_operate(uuid, kSNHelperMessageTypeFlowDivertUUIDRemove);
}

int
snhelper_flow_divert_uuid_policy_clear(void)
{
	int result = 0;
	xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_object_t response;

	xpc_dictionary_set_uint64(request, kSNHelperMessageType, kSNHelperMessageTypeFlowDivertUUIDClear);

	response = copy_response(request);

	if (isa_xpc_dictionary(response)) {
		result = (int)xpc_dictionary_get_int64(response, kSNHelperMessageResult);
	} else {
		result = EINVAL;
	}

	xpc_release(response);
	xpc_release(request);

	return result;
}

int
snhelper_get_uuid_for_app(const char *appID, uuid_t uuid)
{
	int result;
	xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_object_t response;

	xpc_dictionary_set_uint64(request, kSNHelperMessageType, kSNHelperMessageTypeGetUUIDForApp);
	xpc_dictionary_set_string(request, kSNHelperMessageAppID, appID);

	response = copy_response(request);

	if (isa_xpc_dictionary(response)) {
		result = (int)xpc_dictionary_get_int64(response, kSNHelperMessageResult);
		const uint8_t *uuidBytes = xpc_dictionary_get_uuid(response, kSNHelperMessageResultData);
		if (result == 0 && uuid != NULL && uuidBytes != NULL) {
			memcpy(uuid, uuidBytes, sizeof(uuid_t));
		}
	} else {
		result = EINVAL;
	}

	xpc_release(response);
	xpc_release(request);

	return result;
}

static bool
isa_xpc_object_of_type(xpc_object_t obj, xpc_type_t type)
{
	return (obj != NULL && xpc_get_type(obj) == type);
}

bool
isa_xpc_connection(xpc_object_t obj)
{
	return isa_xpc_object_of_type(obj, XPC_TYPE_CONNECTION);
}

bool
isa_xpc_bool(xpc_object_t obj)
{
	return isa_xpc_object_of_type(obj, XPC_TYPE_BOOL);
}

bool
isa_xpc_dictionary(xpc_object_t obj)
{
	return isa_xpc_object_of_type(obj, XPC_TYPE_DICTIONARY);
}

bool
isa_xpc_error(xpc_object_t obj)
{
	return isa_xpc_object_of_type(obj, XPC_TYPE_ERROR);
}

