/*
 * Copyright (c) 2013 Apple Inc.
 * All rights reserved.
 */

#ifndef __SYSTEMCONFIGURATION_SNHELPER_PRIVATE_H__
#define __SYSTEMCONFIGURATION_SNHELPER_PRIVATE_H__

#include <xpc/xpc.h>

#define kSNHelperService		"com.apple.snhelper"

#define kSNHelperMessageType	"message-type"
#define kSNHelperMessageUUID	"uuid"
#define kSNHelperMessageAppID	"app-id"
#define kSNHelperMessageResult	"result-code"
#define kSNHelperMessageResultData "result-data"

enum {
	kSNHelperMessageTypeFlowDivertUUIDAdd,
	kSNHelperMessageTypeFlowDivertUUIDRemove,
	kSNHelperMessageTypeFlowDivertUUIDClear,
	kSNHelperMessageTypeGetUUIDForApp,
};

int snhelper_flow_divert_uuid_policy_add(const uuid_t uuid);
int snhelper_flow_divert_uuid_policy_remove(const uuid_t uuid);
int snhelper_flow_divert_uuid_policy_clear(void);
int snhelper_get_uuid_for_app(const char *, uuid_t);

bool isa_xpc_connection(xpc_object_t obj);
bool isa_xpc_bool(xpc_object_t obj);
bool isa_xpc_dictionary(xpc_object_t obj);
bool isa_xpc_error(xpc_object_t obj);

#endif /* __SYSTEMCONFIGURATION_SNHELPER_PRIVATE_H__ */
