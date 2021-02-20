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

#import "configAgent.h"

@interface DNSAgent:ConfigAgent

@property (nonatomic) AgentType internalAgentType;
@property (nonatomic) NSString *internalAgentName;
@property (nonatomic) AgentSubType internalAgentSubType;

@end

@implementation DNSAgent

@synthesize agentUUID;
@synthesize agentDescription;

+ (NSString *)agentType
{
	return @kConfigAgentTypeDNS;
}

- (instancetype)initWithParameters:(NSDictionary *)parameters
{
	self = [super initWithParameters:parameters];
	if (self) {
		NSString *intf = [parameters valueForKey:@kEntityName];
		NSNumber *subType = [parameters valueForKey:@kAgentSubType];
		NSString *type = [[self class] agentType];

		if ([subType unsignedIntegerValue] == kAgentSubTypeMulticast) {
			type = @kConfigAgentTypeDNSMulticast;
		} else if ([subType unsignedIntegerValue] == kAgentSubTypePrivate) {
			type = @kConfigAgentTypeDNSPrivate;
		}

		_internalAgentName = [NSString stringWithFormat:@"%@-%@", type, intf];
		_internalAgentSubType = [subType unsignedIntegerValue];
		_internalAgentType = kAgentTypeDNS;

		agentDescription = _internalAgentName;
		agentUUID = [super createUUIDForName:agentDescription];
		if (agentUUID == nil) {
			agentUUID = [NSUUID UUID];
		}
	}

	return self;
}

- (AgentType)getAgentType
{
	return _internalAgentType;
}

- (NSString *)getAgentName
{
	return _internalAgentName;
}

- (AgentSubType)getAgentSubType
{
	return _internalAgentSubType;
}

- (NSUUID *)getAgentUUID
{
	return agentUUID;
}

@end