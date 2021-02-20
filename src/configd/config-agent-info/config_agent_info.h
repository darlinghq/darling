/*
 * Copyright (c) 2015, 2016, 2018 Apple Inc. All rights reserved.
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

#ifndef CONFIG_AGENT_INFO_H
#define CONFIG_AGENT_INFO_H

#include <dnsinfo.h>
#include <net/network_agent.h>
#include <xpc/private.h>

__BEGIN_DECLS

#define kConfigAgentDomain                      "SystemConfig"

#define kConfigAgentTypeProxy                   "ProxyAgent"
#define kConfigAgentTypeDNS                     "DNSAgent"

/*
	Returns true for agent with type DNSAgent and domain SystemConfig
 */
boolean_t
is_config_agent_type_dns		(const struct netagent *agent)		API_AVAILABLE(macos(10.12), ios(10.0));

/*
	Returns true for agent with type ProxyAgent and domain SystemConfig
 */
boolean_t
is_config_agent_type_proxy		(const struct netagent *agent)		API_AVAILABLE(macos(10.12), ios(10.0));

/*
	Returns xpc_object_t corresponding to the raw DNSAgent data
		NULL if the agent is NOT a DNSAgent
 */
xpc_object_t
config_agent_copy_dns_information	(const struct netagent	*agentStruct)	API_AVAILABLE(macos(10.12), ios(10.0));

/*
	Returns xpc_object_t (XPC_TYPE_ARRAY) corresponding to the DNS nameservers
		NULL if the agent is NOT a DNSAgent
 */
xpc_object_t
config_agent_get_dns_nameservers	(xpc_object_t resolver)			API_AVAILABLE(macos(10.12), ios(10.0));

/*
	Returns xpc_object_t (XPC_TYPE_ARRAY) corresponding to the DNS search domains
		NULL if the agent is NOT a DNSAgent
 */
xpc_object_t
config_agent_get_dns_searchdomains	(xpc_object_t resolver)			API_AVAILABLE(macos(10.12), ios(10.0));

/*
	Frees the xpc_object_t returned by config_agent_copy_dns_information()
 */
void
config_agent_free_dns_information	(xpc_object_t resolver)			API_AVAILABLE(macos(10.12), ios(10.0));

/*
	Returns xpc_object_t corresponding to the raw ProxyAgent data
		NULL if the agent is NOT a ProxyAgent
 */
xpc_object_t
config_agent_copy_proxy_information	(const struct netagent	*agentStruct)	API_AVAILABLE(macos(10.12), ios(10.0));

/*
	Updates the proxy config with PAC, if applicable. The proxyConfig MUST be
	of type XPC_TYPE_ARRAY containing a XPC_TYPE_DICTIONARY. This format is
	returned by config_agent_copy_proxy_information()

	Returns xpc_object_t to be freed by the caller.
		NULL if the the provided configuration does not need any update.
 */
xpc_object_t
config_agent_update_proxy_information	(xpc_object_t proxyConfig)		API_AVAILABLE(macos(10.12), ios(10.0));

/*
	Frees the xpc_object_t returned by config_agent_copy_proxy_information()
 */
void
config_agent_free_proxy_information	(xpc_object_t proxyConfig)		API_AVAILABLE(macos(10.12), ios(10.0));

__END_DECLS

#endif	/* CONFIG_AGENT_INFO_H */
