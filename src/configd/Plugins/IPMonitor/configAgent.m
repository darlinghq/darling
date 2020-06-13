/*
 * Copyright (c) 2015-2018 Apple Inc. All rights reserved.
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

@interface ConfigAgent()

@property (nonatomic) NWNetworkAgentRegistration * internalRegistrationObject;
@property (nonatomic) NSString *internalAssociatedEntity;
@property (nonatomic, copy) NSData *internalAgentData;
@property (nonatomic) BOOL internalShouldUpdateAgent;
@property (strong) void (^internalStartHandler)(void);
@property (nonatomic) id internalAgentMapping;

@end

@implementation ConfigAgent

@synthesize agentUUID;
@synthesize agentDescription;
@synthesize active;
@synthesize kernelActivated;
@synthesize userActivated;
@synthesize voluntary;
@synthesize specificUseOnly;

+ (NSString *)agentDomain
{
	return @kConfigAgentDomain;
}

+ (NSString *)agentType
{
	return @kConfigAgentTypeGeneric;
}

+ (instancetype)agentFromData:(NSData *)data
{
#pragma unused(data)
	return nil;
}

- (instancetype)initWithParameters:(NSDictionary *)parameters
{
	self = [super init];
	if (self) {
		NSString *intf = [parameters valueForKey:@kEntityName];

		_internalRegistrationObject = nil;
		_internalAssociatedEntity = [intf copy];
		_internalAgentData = nil;
		_internalShouldUpdateAgent = YES;
		_internalAgentMapping = nil;

		active = YES;
		kernelActivated = NO;
		userActivated = YES;
		voluntary = NO;
	}

	return self;
}

- (void)addAgentRegistrationObject:(NWNetworkAgentRegistration *)regObject
{
	_internalRegistrationObject = regObject;
}

- (AgentType)getAgentType
{
	return kAgentTypeUnknown;
}

- (NSUUID *)getAgentUUID
{
	return nil;
}

- (NSString *)getAgentName
{
	return @kConfigAgentTypeGeneric;
}

- (AgentSubType)getAgentSubType
{
	return kAgentSubTypeUnknown;
}

- (NWNetworkAgentRegistration *)getRegistrationObject
{
	return _internalRegistrationObject;
}

- (NSString *)getAssociatedEntity
{
	return _internalAssociatedEntity;
}

- (NSData *)getAgentData
{
	return _internalAgentData;
}

- (NSData *)copyAgentData
{
	return _internalAgentData;
}

- (void)setAgentMapping:(id)agent
{
	_internalAgentMapping = agent;
}

- (id)getAgentMapping
{
	return _internalAgentMapping;
}

- (void)setStartHandler:(void (^)(void))startHandler
{
	if (startHandler != nil) {
		self.internalStartHandler = startHandler;
	}
}

- (BOOL)startAgentWithOptions:(NSDictionary *)options
{
#pragma unused(options)
	BOOL ok = NO;
	if (!self.active) {
		self.active = YES;
		ok = [self.internalRegistrationObject updateNetworkAgent:self];
	}

	return ok;
}

- (void)updateAgentData:(NSData *)data
{
	if ([data isEqual:_internalAgentData]) {
		_internalShouldUpdateAgent = NO;
		return;
	}

	_internalAgentData = [data copy];
	_internalShouldUpdateAgent = YES;
}

- (BOOL)shouldUpdateAgent
{
	return _internalShouldUpdateAgent;
}

- (NSUUID *)createUUIDForName:(NSString *)agentName
{
	/* We would like to have same UUIDs for an interface/domain. So here is a way to fix this,
	 without maintaining any state in configd.

	 - We know that every agent has a unique name.
	 - Use that name to calculate an MD5 hash. MD5 because the digest size is 16 bytes, and so it uuid_t.
	 - create a NSUUID from these raw bytes.

	 - So for a name, we would always have the same UUID.

	 */
	unsigned char hashValue[CC_SHA256_DIGEST_LENGTH];
	const char *strForHash = [agentName UTF8String];
	CC_SHA256(strForHash, (CC_LONG)strlen(strForHash), hashValue);

	return [[NSUUID alloc] initWithUUIDBytes:hashValue];
}

@end
