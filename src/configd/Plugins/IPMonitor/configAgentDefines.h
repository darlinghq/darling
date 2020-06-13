/*
 * Copyright (c) 2015, 2016 Apple Inc. All rights reserved.
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

#ifndef CONFIGAGENTDEFINES_H
#define CONFIGAGENTDEFINES_H

#import "config_agent_info.h"

#define kConfigAgentType                        "AgentType"
#define kConfigAgentTypeGeneric                 "ConfigAgent"

#define kConfigAgentTypeDNSMulticast            kConfigAgentTypeDNS "(m)"
#define kConfigAgentTypeDNSPrivate              kConfigAgentTypeDNS "(p)"

#define kConfigAgentDNSNameServers              "NameServers"
#define kConfigAgentDNSSearchDomains            "SearchDomains"

#define kConfigAgentOutOfBandDataUUID           "OutOfBandDataUUID"
#define kConfigAgentAgentName                   "AgentName"
#define kConfigAgentAgentData                   "AgentData"
#define kConfigAgentAgentUUID                   "AgentUUID"

#endif /* CONFIGAGENTDEFINES_H */
