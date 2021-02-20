/*
 * Copyright (c) 2015-2017 Apple Inc. All rights reserved.
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

#ifndef CONFIG_AGENT_H
#define CONFIG_AGENT_H

#include <net/if.h>
#include <net/network_agent.h>
#include <net/necp.h>
#include <dnsinfo.h>
#include <sys/ioctl.h>
#include <network_information.h>
#include <notify.h>
#include <sys/kern_control.h>
#include <sys/sys_domain.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>

#import <Foundation/Foundation.h>
#import <Network/Network_Private.h>
#import <NetworkExtension/NEPolicySession.h>
#import <CommonCrypto/CommonDigest.h>

#import "configAgentDefines.h"

typedef NS_ENUM(NSUInteger, AgentType) {
	kAgentTypeUnknown = 0,
	kAgentTypeProxy,
	kAgentTypeDNS
};

typedef NS_ENUM(NSUInteger, AgentSubType) {
	kAgentSubTypeUnknown = 0,
	kAgentSubTypeScoped,
	kAgentSubTypeSupplemental,
	kAgentSubTypeDefault,
	kAgentSubTypeMulticast,
	kAgentSubTypePrivate,
	kAgentSubTypeServiceSpecific,
	kAgentSubTypeGlobal,
};

os_log_t	__log_IPMonitor(void);

/* Parameters */
#define kEntityName	"EntityName"
#define kAgentSubType	"AgentSubType"

@interface ConfigAgent : NSObject <NWNetworkAgent>

@property NEPolicySession *preferredPolicySession;

- (instancetype)initWithParameters:(NSDictionary *)parameters;
- (void)addAgentRegistrationObject:(NWNetworkAgentRegistration *)regObject;
- (NWNetworkAgentRegistration *)getRegistrationObject;
- (NSString *)getAssociatedEntity;
- (NSString *)getAgentName;
- (NSData *)getAgentData;
- (AgentType)getAgentType;
- (AgentSubType)getAgentSubType;
- (NSUUID *)getAgentUUID;
- (void)setStartHandler:(void (^)(void))startHandler;
- (BOOL)startAgentWithOptions:(NSDictionary *)options;
- (void)updateAgentData:(NSData *)data;
- (BOOL)shouldUpdateAgent;
- (id)getAgentMapping;
- (void)setAgentMapping:(id)agent;

- (NSUUID *)createUUIDForName:(NSString *)agentName;

@end

#endif /* CONFIG_AGENT_H */
