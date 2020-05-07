/*
 * Copyright (c) 2014-2017 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _NETAGENT_H_
#define _NETAGENT_H_
#include <net/net_kev.h>

#ifdef PRIVATE

#include <netinet/in.h>
#include <sys/socket.h>

#ifdef BSD_KERNEL_PRIVATE
#include <stdbool.h>

errno_t netagent_init(void);
#endif
/*
 * Name registered by the Network Agent kernel control
 */
#define NETAGENT_CONTROL_NAME "com.apple.net.netagent"

struct netagent_message_header {
	u_int8_t                message_type;
	u_int8_t                message_flags;
	u_int32_t               message_id;
	u_int32_t               message_error;
	u_int32_t               message_payload_length;
};

struct netagent_trigger_message {
	u_int32_t               trigger_flags;
	pid_t                   trigger_pid;
	uuid_t                  trigger_proc_uuid;
};

struct netagent_client_message {
	uuid_t                  client_id;
};

struct netagent_assign_nexus_message {
	uuid_t                  assign_client_id;
	u_int8_t                assign_necp_results[0];
};

#define NETAGENT_MESSAGE_TYPE_REGISTER                  1       // Pass netagent to set, no return value
#define NETAGENT_MESSAGE_TYPE_UNREGISTER                2       // No value, no return value
#define NETAGENT_MESSAGE_TYPE_UPDATE                    3       // Pass netagent to update, no return value
#define NETAGENT_MESSAGE_TYPE_GET                               4       // No value, return netagent
#define NETAGENT_MESSAGE_TYPE_TRIGGER                   5       // Kernel initiated, no reply expected
#define NETAGENT_MESSAGE_TYPE_ASSERT                    6       // Deprecated
#define NETAGENT_MESSAGE_TYPE_UNASSERT                  7       // Deprecated
#define NETAGENT_MESSAGE_TYPE_TRIGGER_ASSERT    8       // Kernel initiated, no reply expected
#define NETAGENT_MESSAGE_TYPE_TRIGGER_UNASSERT  9       // Kernel initiated, no reply expected
#define NETAGENT_MESSAGE_TYPE_REQUEST_NEXUS             10      // Kernel initiated, struct netagent_client_message
#define NETAGENT_MESSAGE_TYPE_ASSIGN_NEXUS              11      // Pass struct netagent_assign_nexus_message
#define NETAGENT_MESSAGE_TYPE_CLOSE_NEXUS               12      // Kernel initiated, struct netagent_client_message
#define NETAGENT_MESSAGE_TYPE_CLIENT_TRIGGER    13      // Kernel initiated, struct netagent_client_message
#define NETAGENT_MESSAGE_TYPE_CLIENT_ASSERT             14      // Kernel initiated, struct netagent_client_message
#define NETAGENT_MESSAGE_TYPE_CLIENT_UNASSERT   15      // Kernel initiated, struct netagent_client_message

#define NETAGENT_OPTION_TYPE_REGISTER                   NETAGENT_MESSAGE_TYPE_REGISTER          // Pass netagent to set, no return value
#define NETAGENT_OPTION_TYPE_UNREGISTER                 NETAGENT_MESSAGE_TYPE_UNREGISTER        // No value, no return value
#define NETAGENT_OPTION_TYPE_UPDATE                             NETAGENT_MESSAGE_TYPE_UPDATE            // Pass netagent to update, no return value
#define NETAGENT_OPTION_TYPE_ASSIGN_NEXUS               NETAGENT_MESSAGE_TYPE_ASSIGN_NEXUS      // Pass struct netagent_assign_nexus_message
#define NETAGENT_OPTION_TYPE_USE_COUNT                  16                                                                      // Pass use count to set, get current use count
#define NETAGENT_MESSAGE_TYPE_ABORT_NEXUS               17      // Kernel private

#define NETAGENT_MESSAGE_FLAGS_RESPONSE                 0x01    // Used for acks, errors, and query responses

#define NETAGENT_MESSAGE_ERROR_NONE                     0
#define NETAGENT_MESSAGE_ERROR_INTERNAL                 1
#define NETAGENT_MESSAGE_ERROR_UNKNOWN_TYPE             2
#define NETAGENT_MESSAGE_ERROR_INVALID_DATA             3
#define NETAGENT_MESSAGE_ERROR_NOT_REGISTERED           4
#define NETAGENT_MESSAGE_ERROR_ALREADY_REGISTERED       5
#define NETAGENT_MESSAGE_ERROR_CANNOT_UPDATE            6
#define NETAGENT_MESSAGE_ERROR_CANNOT_ASSIGN            7

#define NETAGENT_DOMAINSIZE             32
#define NETAGENT_TYPESIZE               32
#define NETAGENT_DESCSIZE               128

#define NETAGENT_MAX_DATA_SIZE  4096

#define NETAGENT_FLAG_REGISTERED                0x0001 // Agent is registered
#define NETAGENT_FLAG_ACTIVE                    0x0002 // Agent is active
#define NETAGENT_FLAG_KERNEL_ACTIVATED          0x0004 // Agent can be activated by kernel activity
#define NETAGENT_FLAG_USER_ACTIVATED            0x0008 // Agent can be activated by system call (netagent_trigger)
#define NETAGENT_FLAG_VOLUNTARY                 0x0010 // Use of agent is optional
#define NETAGENT_FLAG_SPECIFIC_USE_ONLY         0x0020 // Agent should only be used and activated when specifically required
#define NETAGENT_FLAG_NETWORK_PROVIDER          0x0040 // Agent provides network access
#define NETAGENT_FLAG_NEXUS_PROVIDER            0x0080 // Agent provides a skywalk nexus
#define NETAGENT_FLAG_SUPPORTS_BROWSE           0x0100 // Assertions will cause agent to fill in browse endpoints
#define NETAGENT_FLAG_REQUIRES_ASSERT           0x0200 // Assertions are expected to be taken against this agent
#define NETAGENT_FLAG_NEXUS_LISTENER            0x0400 // Nexus supports listeners
#define NETAGENT_FLAG_UPDATE_IMMEDIATELY        0x0800 // Updates the clients without waiting for a leeway
#define NETAGENT_FLAG_CUSTOM_ETHER_NEXUS        0x2000 // Agent provides a custom ethertype nexus
#define NETAGENT_FLAG_CUSTOM_IP_NEXUS           0x4000 // Agent provides a custom IP nexus
#define NETAGENT_FLAG_INTERPOSE_NEXUS           0x8000 // Agent provides an interpose nexus
#define NETAGENT_FLAG_SUPPORTS_RESOLVE          0x10000 // Assertions will cause agent to fill in resolved endpoints

#define NETAGENT_NEXUS_MAX_REQUEST_TYPES                        16
#define NETAGENT_NEXUS_MAX_RESOLUTION_TYPE_PAIRS        16

#define NETAGENT_NEXUS_FRAME_TYPE_UNKNOWN               0
#define NETAGENT_NEXUS_FRAME_TYPE_LINK                  1
#define NETAGENT_NEXUS_FRAME_TYPE_INTERNET              2
#define NETAGENT_NEXUS_FRAME_TYPE_TRANSPORT             3
#define NETAGENT_NEXUS_FRAME_TYPE_APPLICATION   4

#define NETAGENT_NEXUS_ENDPOINT_TYPE_ADDRESS    1
#define NETAGENT_NEXUS_ENDPOINT_TYPE_HOST               2
#define NETAGENT_NEXUS_ENDPOINT_TYPE_BONJOUR    3
#define NETAGENT_NEXUS_ENDPOINT_TYPE_SRV        5

#define NETAGENT_NEXUS_FLAG_SUPPORTS_USER_PACKET_POOL   0x1
#define NETAGENT_NEXUS_FLAG_ASSERT_UNSUPPORTED                  0x2 // No calls to assert the agent are required
#define NETAGENT_NEXUS_FLAG_SHOULD_USE_EVENT_RING       0x4 // indicates that nexus agent should use event rings

struct netagent_nexus {
	u_int32_t       frame_type;
	u_int32_t       endpoint_assignment_type;
	u_int32_t       endpoint_request_types[NETAGENT_NEXUS_MAX_REQUEST_TYPES];
	u_int32_t       endpoint_resolution_type_pairs[NETAGENT_NEXUS_MAX_RESOLUTION_TYPE_PAIRS * 2];
	u_int32_t       nexus_flags;
};

#define NETAGENT_TRIGGER_FLAG_USER              0x0001  // Userspace triggered agent
#define NETAGENT_TRIGGER_FLAG_KERNEL            0x0002  // Kernel triggered agent

struct kev_netagent_data {
	uuid_t          netagent_uuid;
};

// To be used with kernel control socket
struct netagent {
	uuid_t          netagent_uuid;
	char            netagent_domain[NETAGENT_DOMAINSIZE];
	char            netagent_type[NETAGENT_TYPESIZE];
	char            netagent_desc[NETAGENT_DESCSIZE];
	u_int32_t       netagent_flags;
	u_int32_t       netagent_data_size;
	u_int8_t        netagent_data[0];
};

// To be used with SIOCGAGENTDATA
struct netagent_req {
	uuid_t          netagent_uuid;
	char            netagent_domain[NETAGENT_DOMAINSIZE];
	char            netagent_type[NETAGENT_TYPESIZE];
	char            netagent_desc[NETAGENT_DESCSIZE];
	u_int32_t       netagent_flags;
	u_int32_t       netagent_data_size;
	u_int8_t        *netagent_data;
};

// To be used with SIOCGAGENTLIST
struct netagentlist_req {
	u_int32_t       data_size;
	u_int8_t        *data;
};
#ifdef BSD_KERNEL_PRIVATE
int netagent_ioctl(u_long cmd, caddr_t data);

struct netagent_req32 {
	uuid_t          netagent_uuid;
	char            netagent_domain[NETAGENT_DOMAINSIZE];
	char            netagent_type[NETAGENT_TYPESIZE];
	char            netagent_desc[NETAGENT_DESCSIZE];
	u_int32_t       netagent_flags;
	u_int32_t       netagent_data_size;
	user32_addr_t   netagent_data;
};
struct netagent_req64 {
	uuid_t          netagent_uuid;
	char            netagent_domain[NETAGENT_DOMAINSIZE];
	char            netagent_type[NETAGENT_TYPESIZE];
	char            netagent_desc[NETAGENT_DESCSIZE];
	u_int32_t       netagent_flags;
	u_int32_t       netagent_data_size;
	user64_addr_t   netagent_data __attribute__((aligned(8)));
};
struct netagentlist_req32 {
	u_int32_t       data_size;
	user32_addr_t   data;
};
struct netagentlist_req64 {
	u_int32_t       data_size;
	user64_addr_t   data __attribute__((aligned(8)));
};

struct necp_client_agent_parameters;

// Kernel accessors
extern void netagent_post_updated_interfaces(uuid_t uuid); // To be called from interface ioctls

extern u_int32_t netagent_get_flags(uuid_t uuid);

extern errno_t netagent_set_flags(uuid_t uuid, u_int32_t flags);

extern u_int32_t netagent_get_generation(uuid_t uuid);

extern bool netagent_get_agent_domain_and_type(uuid_t uuid, char *domain, char *type);

extern int netagent_kernel_trigger(uuid_t uuid);

extern int netagent_client_message(uuid_t agent_uuid, uuid_t necp_client_uuid, pid_t pid, void *handle, u_int8_t message_type);

extern int netagent_client_message_with_params(uuid_t agent_uuid,
    uuid_t necp_client_uuid,
    pid_t pid,
    void *handle,
    u_int8_t message_type,
    struct necp_client_agent_parameters *parameters,
    void **assigned_results,
    size_t *assigned_results_length);

extern int netagent_copyout(uuid_t uuid, user_addr_t user_addr, u_int32_t user_size);


// Kernel agent management

typedef void * netagent_session_t;

struct netagent_nexus_agent {
	struct netagent                         agent;
	struct netagent_nexus           nexus_data;
};

#define NETAGENT_EVENT_TRIGGER                                  NETAGENT_MESSAGE_TYPE_CLIENT_TRIGGER
#define NETAGENT_EVENT_ASSERT                                   NETAGENT_MESSAGE_TYPE_CLIENT_ASSERT
#define NETAGENT_EVENT_UNASSERT                                 NETAGENT_MESSAGE_TYPE_CLIENT_UNASSERT
#define NETAGENT_EVENT_NEXUS_FLOW_INSERT                        NETAGENT_MESSAGE_TYPE_REQUEST_NEXUS
#define NETAGENT_EVENT_NEXUS_FLOW_REMOVE                        NETAGENT_MESSAGE_TYPE_CLOSE_NEXUS
#define NETAGENT_EVENT_NEXUS_FLOW_ABORT                         NETAGENT_MESSAGE_TYPE_ABORT_NEXUS

typedef errno_t (*netagent_event_f)(u_int8_t event, uuid_t necp_client_uuid, pid_t pid, void *necp_handle, void *context, struct necp_client_agent_parameters *parameters, void **assigned_results, size_t *assigned_results_length);

extern netagent_session_t netagent_create(netagent_event_f event_handler, void *handle);

extern void netagent_destroy(netagent_session_t session);

extern errno_t netagent_register(netagent_session_t session, struct netagent *agent);

extern errno_t netagent_update(netagent_session_t session, struct netagent *agent);

extern errno_t netagent_unregister(netagent_session_t session);

extern errno_t netagent_assign_nexus(netagent_session_t _session,
    uuid_t necp_client_uuid,
    void *assign_message,
    size_t assigned_results_length);                                                                      // Length of assigned_results_length

extern errno_t netagent_update_flow_protoctl_event(netagent_session_t _session,
    uuid_t client_id,
    uint32_t protoctl_event_code,
    uint32_t protoctl_event_val,
    uint32_t protoctl_event_tcp_seq_number);

extern int netagent_use(uuid_t agent_uuid, uint64_t *out_use_count);

#endif /* BSD_KERNEL_PRIVATE */

#ifndef KERNEL
extern int netagent_trigger(uuid_t agent_uuid, size_t agent_uuidlen);
#endif /* !KERNEL */

#endif /* PRIVATE */

#endif /* _NETAGENT_H_ */
