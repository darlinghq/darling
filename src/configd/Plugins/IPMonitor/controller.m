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

#import "controller.h"
#import <SystemConfiguration/SCPrivate.h>
#import "ip_plugin.h"

#define numberToNSNumber(x)	[NSNumber numberWithUnsignedInteger:x]

#define dnsAgentDefault		"_defaultDNS"
#define proxyAgentDefault	"_defaultProxy"
#define multipleEntitySuffix	" #"
#define prefixForInterfaceName	"@"

/* These define the starting and ending order of each policy section */
#define INIT_ORDER_FOR_SCOPED_INTERFACE_POLICY		100
#define INIT_ORDER_FOR_DOMAIN_POLICY			500
#define INIT_ORDER_FOR_DEFAULT_POLICY			1000

#define SKIP_ORDER_FOR_SCOPED_INTERFACE_POLICY		250
#define SKIP_ORDER_FOR_DOMAIN_POLICY			750
#define SKIP_ORDER_FOR_DEFAULT_POLICY			1250

#define POLICY_TYPE_NO_POLICY				-1
#define CONFIG_AGENT_DATA_LIMIT				MIN(NETAGENT_MAX_DATA_SIZE, 1024)

typedef struct resolverList {
	dns_resolver_t	**default_resolvers;
	uint32_t	n_default_resolvers;
	dns_resolver_t  **multicast_resolvers;
	uint32_t	n_multicast_resolvers;
	dns_resolver_t	**private_resolvers;
	uint32_t	n_private_resolvers;
} resolver_list_t;

@interface AgentController()

@property (nonatomic) NSMutableDictionary	*	floatingProxyAgentList;
@property (nonatomic) NSMutableDictionary	*	floatingDNSAgentList;
@property (nonatomic) NSMutableDictionary	*	policyDB;
@property (nonatomic) NEPolicySession		*	policySession;
@property (nonatomic) NEPolicySession		*	controlPolicySession;

@end

@implementation AgentController

#pragma mark Init

+ (AgentController *)sharedController
{
	static AgentController * gController = nil;
	static dispatch_once_t onceToken;

	dispatch_once(&onceToken, ^{
		gController = [[AgentController alloc] init];
	});

	@synchronized (gController) {
		if (![gController isControllerReady]) {
			if (![gController initializeController]) {
				return nil;
			}
		}
	}

	return gController;
}

- (instancetype)init
{
	self = [super init];
	if (self) {
		[self initializeController];
	}

	return self;
}

- (BOOL)initializeController
{
	const char	*errorMessage	= NULL;

	do {
		/*	The NE policy session for the controller */

		if (self.policySession == nil) {
			self.policySession = [self createPolicySession];
			if (self.policySession == nil) {
				errorMessage = "Failed to create a policy session";
				break;
			}
		}

		/*	A dictionary of all floating proxy agents
		 *		Key	:	<entity-name> (can be an interface name or domain name)
		 *		Value	:	agent object
		 */

		if (self.floatingProxyAgentList == nil) {
			self.floatingProxyAgentList = [NSMutableDictionary dictionary];
			if (self.floatingProxyAgentList == nil) {
				errorMessage = "Failed to create a dictionary";
				break;
			}
		}

		/*	A dictionary of all floating dns agents
		 *		Key	:	<entity-name> (can be an interface name or domain name)
		 *		Value	:	agent object
		 */

		if (self.floatingDNSAgentList == nil) {
			self.floatingDNSAgentList = [NSMutableDictionary dictionary];
			if (self.floatingDNSAgentList == nil) {
				errorMessage = "Failed to create a dictionary";
				break;
			}
		}

		/*	A dictionary for the maintaining the policy IDs for all installed policy.
		 *	These IDs would be necessary to uninstall a policy when an agent goes away
		 *		Key	:	agent name (which can be retrieved by [agent getAgentName])
		 *		Value	:	An array of integers, each being a policy ID for that agent
		 */

		if (self.policyDB == nil) {
			self.policyDB = [NSMutableDictionary dictionary];
			if (self.policyDB == nil) {
				errorMessage = "Failed to create a dictionary";
				break;
			}
		}

		/*	The queue to run the all processing on */

		if (self.controllerQueue == nil) {
			self.controllerQueue = dispatch_queue_create("com.apple.SystemConfiguration.controllerQueue", NULL);
			if (self.controllerQueue == nil) {
				errorMessage = "Failed to create a queue";
				break;
			}
		}
	} while (0);

	if (errorMessage != NULL) {
		/* Some error occurred. This is unlikely during controller initialization... */
		SC_log(LOG_ERR, "Error occured while initializing AgentController: %s", errorMessage);
		_SC_crash(errorMessage, NULL, NULL);
		return NO;
	}

	return YES;
}

- (NEPolicySession *)createPolicySession
{
	return [[NEPolicySession alloc] init];
}

- (BOOL)isControllerReady
{
	/* Make sure that we have all our data structures in place */
	return ((self.policySession != nil) &&
		(self.floatingProxyAgentList != nil) &&
		(self.floatingDNSAgentList != nil) &&
		(self.policyDB != nil) &&
		(self.controllerQueue != nil));
}

/* ========================== proxy agent helpers =========================== */
#pragma mark Proxy agent helper functions

- (NSData *)dataForProxyArray:(CFArrayRef)proxy_array_for_data
{
	CFDataRef data = NULL;
	(void)_SCSerialize(proxy_array_for_data, &data, NULL, NULL);
	return (__bridge_transfer NSData *)data;
}

- (NSData *)dataForProxyDictionary:(CFDictionaryRef)domain_proxy
{
	NSData	*	data = nil;
	CFMutableDictionaryRef domain_proxy_dict;

	if (domain_proxy == NULL) {
		SC_log(LOG_NOTICE, "Invalid domain proxy dict");
		return nil;
	}

	domain_proxy_dict = CFDictionaryCreateMutableCopy(NULL, 0, domain_proxy);
	CFDictionaryRemoveValue(domain_proxy_dict, kSCPropNetProxiesSupplementalMatchDomain);

	data = (__bridge_transfer NSData *)(SCNetworkProxiesCreateProxyAgentData(domain_proxy_dict));
	CFRelease(domain_proxy_dict);

	return data;
}

- (NSData *)getProxyDataFromCurrentConfig:(CFDictionaryRef)proxies
				   domain:(NSString *)domain
{
	CFIndex		count;
	CFIndex		idx;
	CFArrayRef	supplemental;

	if (proxies == NULL || domain == nil) {
		SC_log(LOG_NOTICE, "Invalid proxies/domain");
		return nil;
	}

	supplemental = CFDictionaryGetValue(proxies, kSCPropNetProxiesSupplemental);
	count = supplemental ? CFArrayGetCount(supplemental) : 0;

	for (idx = 0; idx < count; idx++) {
		CFDictionaryRef	domain_proxy;
		CFStringRef	match_domain;

		domain_proxy = CFArrayGetValueAtIndex(supplemental, idx);
		match_domain = CFDictionaryGetValue(domain_proxy, kSCPropNetProxiesSupplementalMatchDomain);
		if (match_domain != NULL && CFEqual(match_domain, (__bridge CFTypeRef)(domain))) {
			return [self dataForProxyDictionary:domain_proxy];
		}
	}

	return nil;
}

- (bool)getIntValue:(CFTypeRef)cf_value
	   valuePtr:(int *) int_value_ptr
{
	bool valid = false;
	if (cf_value && CFGetTypeID(cf_value) == CFNumberGetTypeID() && CFNumberGetValue(cf_value, kCFNumberIntType, int_value_ptr))
	{
		valid = true;
	}
	return valid;
}

- (int)countProxyEntriesEnabled:(CFDictionaryRef)proxies
{
	int	enabled = 0;

	if (proxies == NULL) {
		SC_log(LOG_NOTICE, "Invalid proxies");
		return 0;
	}

	if (([self getIntValue:CFDictionaryGetValue(proxies, kSCPropNetProxiesHTTPEnable) valuePtr:&enabled] && enabled > 0)	||
	    ([self getIntValue:CFDictionaryGetValue(proxies, kSCPropNetProxiesHTTPSEnable) valuePtr:&enabled] && enabled > 0)	||
	    ([self getIntValue:CFDictionaryGetValue(proxies, kSCPropNetProxiesProxyAutoConfigEnable) valuePtr:&enabled] && enabled > 0) ||
	    ([self getIntValue:CFDictionaryGetValue(proxies, kSCPropNetProxiesFTPEnable) valuePtr:&enabled] && enabled > 0)	||
	    ([self getIntValue:CFDictionaryGetValue(proxies, kSCPropNetProxiesGopherEnable) valuePtr:&enabled] && enabled > 0)	||
	    ([self getIntValue:CFDictionaryGetValue(proxies, kSCPropNetProxiesRTSPEnable) valuePtr:&enabled] && enabled > 0)	||
	    ([self getIntValue:CFDictionaryGetValue(proxies, kSCPropNetProxiesSOCKSEnable) valuePtr:&enabled] && enabled > 0)	||
	    ([self getIntValue:CFDictionaryGetValue(proxies, kSCPropNetProxiesProxyAutoDiscoveryEnable) valuePtr:&enabled] && enabled > 0)) {
		return enabled;
	}

	return 0;
}

- (void)processSupplementalProxyChanges:(CFDictionaryRef)proxies
{
	CFIndex			count;
	NSMutableArray	*	deleteList;
	NSCountedSet	*	duplicate_domain_list;
	CFIndex			idx;
	NSMutableArray	*	new_domain_list;
	NSMutableArray	*	old_domain_list;
	CFArrayRef		supplemental;
	NSMutableArray	*	update_agent_list;

	if (proxies == NULL) {
		SC_log(LOG_INFO, "No proxy config to process");
		return;
	}

	old_domain_list = [self getAgentList:self.floatingProxyAgentList
				   agentType:kAgentTypeProxy
				agentSubType:kAgentSubTypeSupplemental];
	duplicate_domain_list = [[NSCountedSet alloc] initWithCapacity:0];
	new_domain_list = [NSMutableArray array];
	update_agent_list = [NSMutableArray array];
	supplemental = CFDictionaryGetValue(proxies, kSCPropNetProxiesSupplemental);
	count = supplemental ? CFArrayGetCount(supplemental) : 0;
	deleteList = [NSMutableArray array];

	for (idx = 0; idx < count; idx++) {
		CFDictionaryRef		domain_proxy;
		CFStringRef		match_domain;
		int			proxy_count;

		domain_proxy = CFArrayGetValueAtIndex(supplemental, idx);
		match_domain = CFDictionaryGetValue(domain_proxy, kSCPropNetProxiesSupplementalMatchDomain);
		if (match_domain == NULL) {
			continue;
		}

		/*	This domain is present in current config. But if it has generic (no protocols enabled)
		 *	proxy content, there is no real use of that agent. Do NOT add it to
		 *	the new_domain_list.
		 *
		 *	This way, if there was an agent previously for this domain,
		 *	it will be destroyed AND since it is not present in the new domain list, we wont
		 *	spawn a new agent too! :)
		 */

		proxy_count = [self countProxyEntriesEnabled:domain_proxy];
		if (proxy_count == 0) {
			SC_log(LOG_INFO, "Proxy settings on %@ are generic. Not recognizing as new domain", match_domain);
			continue;
		}

		[new_domain_list addObject:(__bridge NSString *)match_domain];
	}

	[self cleanConflictingAgentsFromList:old_domain_list
				    new_list:new_domain_list
			     agentDictionary:self.floatingProxyAgentList];

	for (NSString *key in old_domain_list) {
		BOOL domain_present;

		domain_present = [new_domain_list containsObject:key];
		if (domain_present == NO) {
			id agent;

			agent = [self.floatingProxyAgentList objectForKey:key];
			[self destroyFloatingAgent:agent];
		}
	}

	/*	At this point, whatever is in the controller's floating agent list,
	 *	is present in the current proxy config. The current proxy config
	 *	might have even more configs, not known to the controller, YET
	 */

	for (NSString *domain in old_domain_list) {
		id	agent;
		id	mapped_agent;

		agent = [self.floatingProxyAgentList objectForKey:domain];
		if (agent == nil) {
			continue;
		}

		/* Am I mapped to some agent? */
		mapped_agent = [agent getAgentMapping];
		if (mapped_agent) {
			/*	OK, this agent is mapped to some other agent. We compare this agent's data
			 *	to the current data of the agent to which it is mapped. If different, we destroy
			 *	the agent and later map it to someone else OR spawn a new one.
			 */
			NSData	*	mapped_agent_data;

			mapped_agent_data = [self getProxyDataFromCurrentConfig:proxies domain:[mapped_agent getAssociatedEntity]];
			if (mapped_agent_data == nil || ![[agent getAgentData] isEqual:mapped_agent_data]) {
				/* Something changed for mapped agent */
				[deleteList addObject:agent];
				continue;
			}
		} else {
			/*	Since this agent is NOT mapped to any other agent, this agent is
			 *	registered with the kernel. So instead of destroying the agent and
			 *	re-registering it, just update it here.
			 *
			 *	All the agents which were mapped to this agent, will be deleted and
			 *	re-mapped, if the data changed.
			 */
			NSData	*	agent_data;

			agent_data = [self getProxyDataFromCurrentConfig:proxies domain:[agent getAssociatedEntity]];
			if (![[agent getAgentData] isEqual:agent_data]) {
				/* Something changed for agent */
				[agent updateAgentData:agent_data];

				/*	The reason I don't publish the data to agent here is that, if there were
				 *	some agents mapping to this one, they will momentarily have a policy for
				 *	using this agent UUID for some domain based on this agent's previous data.
				 */
				[update_agent_list addObject:agent];
			}
		}
		[new_domain_list removeObject:domain];
	}

	for (id agent in deleteList) {
		SC_log(LOG_INFO, "Destroying agent %@ because something changed!", [agent getAgentName]);
		[self destroyFloatingAgent:agent];
	}

	for (id agent in update_agent_list) {
		[self publishToAgent:agent];
	}

	for (idx = 0; idx < count; idx++) {
		CFDictionaryRef		domain_proxy;
		CFStringRef		match_domain;

		domain_proxy = CFArrayGetValueAtIndex(supplemental, idx);
		match_domain = CFDictionaryGetValue(domain_proxy, kSCPropNetProxiesSupplementalMatchDomain);

		if (match_domain != NULL) {
			NSData		*	data;
			NSUInteger		found;
			id			mapped_agent;

			found = [new_domain_list indexOfObject:(__bridge id _Nonnull)(match_domain)];
			if (found == NSNotFound) {
				continue;
			}

			/*
			 *	We will only process agents which are mapped AND the agent they were mapped to, changed OR
			 *	agents for domains which we did not know before.
			 */

			NSUInteger domainInstance = [duplicate_domain_list countForObject:(__bridge id _Nonnull)(match_domain)];
			if (domainInstance > 0) {
				/* domainInstance will be > 0, only if we have conflicting domains */
				domainInstance++;
				NSString *ns_domain_name_copy = [NSString stringWithFormat:@"%@" multipleEntitySuffix "%lu", match_domain, (unsigned long)domainInstance];

				data = [self dataForProxyDictionary:domain_proxy];

				BOOL ok = [self spawnFloatingAgent:[ProxyAgent class]
							entity:ns_domain_name_copy
							agentSubType:kAgentSubTypeSupplemental
							addPolicyOfType:NEPolicyConditionTypeDomain
							publishData:data];
				if (ok) {
					id agent = [self.floatingProxyAgentList objectForKey:ns_domain_name_copy];
					SC_log(LOG_INFO, "Duplicate Proxy agent %@", [agent getAgentName]);;
				}
			} else {
				data = [self dataForProxyDictionary:domain_proxy];
				mapped_agent = [self getAgentWithSameDataAndSubType:self.floatingProxyAgentList
									       data:data
									    subType:kAgentSubTypeSupplemental];
				if (mapped_agent != nil) {
					[self spawnMappedFloatingAgent:mapped_agent
							entity:(__bridge NSString *)(match_domain)
							agentSubType:kAgentSubTypeSupplemental
							addPolicyOfType:NEPolicyConditionTypeDomain
							updateData:data];
				} else {
					[self spawnFloatingAgent:[ProxyAgent class]
							entity:(__bridge NSString *)(match_domain)
							agentSubType:kAgentSubTypeSupplemental
							addPolicyOfType:NEPolicyConditionTypeDomain
							publishData:data];
				}
			}

			[new_domain_list removeObjectAtIndex:found];
			[duplicate_domain_list addObject:(__bridge id _Nonnull)(match_domain)];
		}
	}

	return;
}

- (void)processScopedProxyChanges:(CFDictionaryRef)proxies
{
	NSMutableArray		*	old_intf_list;
	CFDictionaryRef			scoped_proxies;
	CFIndex				scoped_proxies_count;

	old_intf_list = [self getAgentList:self.floatingProxyAgentList
				 agentType:kAgentTypeProxy
			      agentSubType:kAgentSubTypeScoped];

	scoped_proxies = CFDictionaryGetValue(proxies, kSCPropNetProxiesScoped);
	scoped_proxies_count =	scoped_proxies ? CFDictionaryGetCount(scoped_proxies) : 0;

	if (scoped_proxies_count > 0) {
		const void **keys;

		keys = malloc(scoped_proxies_count * sizeof(void *));
		CFDictionaryGetKeysAndValues(scoped_proxies, keys, NULL);

		for (int i = 0; i < scoped_proxies_count; i++) {
			NSData		*	data = nil;
			NSUInteger		idx;
			CFArrayRef		matching;
			NSString	*	ns_if_name;
			NSString	*	ns_if_name_with_prefix;
			int			proxy_count = 0;
			id			proxyAgent;

			ns_if_name = (__bridge NSString *)keys[i];
			ns_if_name_with_prefix = [NSString stringWithFormat:@"%s%@", prefixForInterfaceName, ns_if_name];

			/* Does the proxy config have any protocols enabled? */
			proxy_count = [self countProxyEntriesEnabled:CFDictionaryGetValue(scoped_proxies,
												(__bridge const void *)(ns_if_name))];

			if (proxy_count == 0) {
				SC_log(LOG_INFO, "Proxy settings on %@ are generic. Skipping", ns_if_name);
				continue;
			}

			idx = [old_intf_list indexOfObject:ns_if_name_with_prefix];

			matching = SCNetworkProxiesCopyMatching(proxies, NULL, (__bridge CFStringRef)(ns_if_name));
			if (matching != NULL) {
				data = [self dataForProxyArray:matching];
				CFRelease(matching);
			}

			if (idx == NSNotFound) {
				/* We need to spawn an agent */
				[self spawnFloatingAgent:[ProxyAgent class]
						entity:ns_if_name_with_prefix
						agentSubType:kAgentSubTypeScoped
						addPolicyOfType:NEPolicyConditionTypeScopedInterface
						publishData:data];

				continue;
			} else {
				/* We have an agent for this interface. Update it */
				[old_intf_list removeObjectAtIndex:idx];
			}

			proxyAgent = [self.floatingProxyAgentList objectForKey:ns_if_name_with_prefix];
			if (proxyAgent != nil) {
				/* Do we need to update this agent? */
				[proxyAgent updateAgentData:data];
				if ([proxyAgent shouldUpdateAgent]) {
					[self publishToAgent:proxyAgent];
				}
			}
		}

		free(keys);
	}

	[self deleteAgentList:self.floatingProxyAgentList list:old_intf_list];
}

- (void)processServiceSpecificProxyChanges:(CFDictionaryRef)proxies
{
	NSMutableArray		*	old_service_list;
	CFDictionaryRef			service_proxies;
	CFIndex				service_proxies_count;

	old_service_list = [self getAgentList:self.floatingProxyAgentList
				    agentType:kAgentTypeProxy
				agentSubType:kAgentSubTypeServiceSpecific];

	service_proxies = CFDictionaryGetValue(proxies, kSCPropNetProxiesServices);
	service_proxies_count =	service_proxies ? CFDictionaryGetCount(service_proxies) : 0;

	if (service_proxies_count > 0) {
		const void **keys;

		keys = malloc(service_proxies_count * sizeof(void *));
		CFDictionaryGetKeysAndValues(service_proxies, keys, NULL);

		for (int i = 0; i < service_proxies_count; i++) {
			NSData		*	data = nil;
			NSUInteger		idx;
			NSString	*	ns_service_identifier = nil;
			NSString	*	ns_service_with_prefix = nil;
			int			proxy_count = 0;
			id			proxyAgent;
			CFDictionaryRef		proxyDict = NULL;

			ns_service_identifier = (__bridge NSString *)keys[i];
			ns_service_with_prefix = [NSString stringWithFormat:@"%s%@", prefixForInterfaceName, ns_service_identifier];

			/* Does the proxy config have any protocols enabled? */
			proxy_count = [self countProxyEntriesEnabled:CFDictionaryGetValue(service_proxies,
											  (__bridge const void *)(ns_service_identifier))];

			if (proxy_count == 0) {
				SC_log(LOG_INFO, "Proxy settings on %@ are generic. Skipping", ns_service_identifier);
				continue;
			}

			proxyDict = CFDictionaryGetValue(service_proxies, (__bridge CFStringRef)ns_service_identifier);
			if (proxyDict != nil) {
				data = [self dataForProxyArray:(__bridge CFArrayRef)(@[ (__bridge NSDictionary *)proxyDict ])];
			}

			idx = [old_service_list indexOfObject:ns_service_with_prefix];
			if (idx == NSNotFound) {
				/* We need to spawn an agent */
				[self spawnFloatingAgent:[ProxyAgent class]
						  entity:ns_service_with_prefix
					    agentSubType:kAgentSubTypeServiceSpecific
					 addPolicyOfType:(POLICY_TYPE_NO_POLICY) /* Don't install a policy */
					     publishData:data];

				continue;
			} else {
				/* We have an agent for this service. Update it */
				[old_service_list removeObjectAtIndex:idx];
			}

			proxyAgent = [self.floatingProxyAgentList objectForKey:ns_service_with_prefix];
			if (proxyAgent != nil) {
				/* Do we need to update this agent? */
				[proxyAgent updateAgentData:data];
				if ([proxyAgent shouldUpdateAgent]) {
					[self publishToAgent:proxyAgent];
				}
			}
		}

		free(keys);
	}

	[self deleteAgentList:self.floatingProxyAgentList list:old_service_list];
}

- (BOOL)isGlobalProxy:(CFDictionaryRef)proxies
{
	if (CFDictionaryContainsKey(proxies, kSCPropNetProxiesBypassAllowed)) {
		/*
		 * Since we did not ask to "bypass" the proxies, this key will always
		 * be present in a managed (global) proxy configuration
		 */
		return YES;
	}

	return NO;
}

- (void)processDefaultProxyChanges:(CFDictionaryRef)proxies
{
	CFArrayRef			global_proxy;
	CFIndex				global_proxy_count;
	CFMutableDictionaryRef		proxies_copy;

	proxies_copy = CFDictionaryCreateMutableCopy(NULL, 0, proxies);
	CFDictionaryRemoveValue(proxies_copy, kSCPropNetProxiesScoped);
	CFDictionaryRemoveValue(proxies_copy, kSCPropNetProxiesServices);
	CFDictionaryRemoveValue(proxies_copy, kSCPropNetProxiesSupplemental);

	global_proxy = CFArrayCreate(NULL, (const void **)&proxies_copy, 1, &kCFTypeArrayCallBacks);
	global_proxy_count = CFArrayGetCount(global_proxy);
	if (global_proxy_count > 0 &&
	    [self countProxyEntriesEnabled:proxies_copy] == 0) {
		SC_log(LOG_INFO, "Proxy settings on defaultProxy are generic. Skipping");
		global_proxy_count = 0;
	}
	CFRelease(proxies_copy);

	if (global_proxy_count > 0) {
		BOOL		spawnAgent = YES;
		id		proxyAgent;
		NSData *	data;

		data = [self dataForProxyArray:global_proxy];
		proxyAgent = [self.floatingProxyAgentList objectForKey:@proxyAgentDefault];
		if (proxyAgent != nil) {
			if (![data isEqual:[proxyAgent getAgentData]]) {
				[self destroyFloatingAgent:proxyAgent];
			} else {
				spawnAgent = NO;
			}
		}

		if (spawnAgent) {
			AgentSubType subtype = kAgentSubTypeDefault;
			NEPolicyConditionType conditionType = NEPolicyConditionTypeNone;
			if ([self isGlobalProxy:proxies_copy]) {
				SC_log(LOG_INFO, "Global proxy detected...");
				conditionType = NEPolicyConditionTypeAllInterfaces;
				subtype = kAgentSubTypeGlobal;
			}

			[self spawnFloatingAgent:[ProxyAgent class]
					entity:@proxyAgentDefault
					agentSubType:subtype
					addPolicyOfType:conditionType
					publishData:data];
		}
	} else {
		/* No default proxy config OR generic (no protocols enabled) default proxy config.
		 * Destroy the default agent if we had one
		 */
		id	proxyAgent;

		proxyAgent = [self.floatingProxyAgentList objectForKey:@proxyAgentDefault];
		if (proxyAgent != nil) {
			[self destroyFloatingAgent:proxyAgent];
		}
	}

	CFRelease(global_proxy);
}

- (void)processProxyChanges
{
	CFDictionaryRef			proxies;

	proxies = SCDynamicStoreCopyProxiesWithOptions(NULL, NULL);
	if (proxies == NULL) {
		SC_log(LOG_INFO, "No proxy information");

		NSMutableDictionary *copy = [self.floatingProxyAgentList copy];
		for (NSString *entity in copy) {
			id agent = [copy objectForKey:entity];
			[self destroyFloatingAgent:agent];
		}

		return;
	}

	[self processDefaultProxyChanges:proxies];
	[self processScopedProxyChanges:proxies];
	[self processSupplementalProxyChanges:proxies];
	[self processServiceSpecificProxyChanges:proxies];

	CFRelease(proxies);
}

/* ========================== DNS agent helpers =========================== */
#pragma mark DNS agent helper functions

- (void)freeResolverList:(resolver_list_t *)resolvers
{
	/*	This is a shallow free of resolver_list_t only.
	 *	The actual resolver pointers are owned by 'dns_config'
	 */
	if (resolvers == NULL) {
		return;
	}

	if (resolvers->default_resolvers != NULL) {
		free(resolvers->default_resolvers);
	}
	if (resolvers->multicast_resolvers != NULL) {
		free(resolvers->multicast_resolvers);
	}
	if (resolvers->private_resolvers != NULL) {
		free(resolvers->private_resolvers);
	}

	free(resolvers);
}

- (resolver_list_t *)copyResolverList:(dns_config_t *)dns_config
{
	resolver_list_t	*resolvers	= NULL;

	if ((dns_config->n_resolver > 0) && (dns_config->resolver != NULL)) {
		uint32_t	a	= 0;
		uint32_t	b	= 0;
		uint32_t	c	= 0;

		resolvers = calloc(1, sizeof(resolver_list_t));
		for (int i = 0; i < dns_config->n_resolver; i++) {
			dns_resolver_t	*r	= dns_config->resolver[i];

			if ([self isResolverMulticast:r]) {
				resolvers->n_multicast_resolvers++;
				continue;

			} else if ([self isResolverPrivate:r]) {
				resolvers->n_private_resolvers++;
				continue;
			}

			// do not consider default resolvers with no nameservers
			if (r->domain == NULL && r->n_nameserver > 0) {
				resolvers->n_default_resolvers++;
			}
		}

		SC_log(LOG_INFO, "Resolvers: %d default, %d multicast, %d private",
		      resolvers->n_default_resolvers,
		      resolvers->n_multicast_resolvers,
		      resolvers->n_private_resolvers);

		if (resolvers->n_default_resolvers > 0) {
			resolvers->default_resolvers = calloc(resolvers->n_default_resolvers,
							      sizeof(dns_resolver_t *));
		}
		if (resolvers->n_multicast_resolvers > 0) {
			resolvers->multicast_resolvers = calloc(resolvers->n_multicast_resolvers,
								sizeof(dns_resolver_t *));
		}
		if (resolvers->n_private_resolvers > 0) {
			resolvers->private_resolvers = calloc(resolvers->n_private_resolvers,
							      sizeof(dns_resolver_t *));
		}

		for (int i = 0; i < dns_config->n_resolver; i++) {
			dns_resolver_t	*r	= dns_config->resolver[i];

			if ([self isResolverMulticast:r] &&
			    (a < resolvers->n_multicast_resolvers)) {
				resolvers->multicast_resolvers[a++] = r;
				continue;

			} else if ([self isResolverPrivate:r] &&
				   (b < resolvers->n_private_resolvers)) {
				resolvers->private_resolvers[b++] = r;
				continue;
			}

			if ((r->domain == NULL) &&
			    (r->n_nameserver > 0) &&
			    (c < resolvers->n_default_resolvers)) {
				resolvers->default_resolvers[c++] = r;
			}
		}
	}

	return resolvers;
}

/*
 *	Generate a data blob for the resolver.
 *	Currently the blob only has:
 *		- nameserver count
 *		- sockaddr structs for each nameserver
 *		- ifindex
 */

- (NSData *)dataForResolver:(dns_resolver_t *)resolver
{
	NSData		*	data = nil;
	CFMutableDictionaryRef	resolverDict = nil;

	if (resolver == NULL) {
		SC_log(LOG_NOTICE, "Invalid dns resolver");
		return nil;
	}

	if (resolver->n_search > 0) {
		if (resolverDict == nil) {
			resolverDict = CFDictionaryCreateMutable(NULL,
								 0,
								 &kCFTypeDictionaryKeyCallBacks,
								 &kCFTypeDictionaryValueCallBacks);
		}

		CFMutableArrayRef searchDomainArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

		/* Append search domains */
		for (int i = 0; i < resolver->n_search; i++) {
			CFArrayAppendValue(searchDomainArray, (__bridge CFStringRef)(@(resolver->search[i])));
		}

		CFDictionaryAddValue(resolverDict, CFSTR(kConfigAgentDNSSearchDomains), searchDomainArray);
		CFRelease(searchDomainArray);
	}

	/* Get the count of nameservers */
	if (resolver->n_nameserver > 0) {
		if (resolverDict == nil) {
			resolverDict = CFDictionaryCreateMutable(NULL,
								 0,
								 &kCFTypeDictionaryKeyCallBacks,
								 &kCFTypeDictionaryValueCallBacks);
		}

		CFMutableArrayRef nameserverArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

		/* Get all the nameservers */
		for (int i = 0; i < resolver->n_nameserver; i++) {
			char buf[128] = {0};
			_SC_sockaddr_to_string(resolver->nameserver[i], buf, sizeof(buf));
			if (*buf != '\0') {
				CFArrayAppendValue(nameserverArray, (__bridge CFStringRef)(@(buf)));
			}
		}

		CFDictionaryAddValue(resolverDict, CFSTR(kConfigAgentDNSNameServers), nameserverArray);
		CFRelease(nameserverArray);
	}

	if (resolverDict != nil) {
		data = [NSPropertyListSerialization dataWithPropertyList:(__bridge id _Nonnull)(resolverDict)
								  format:NSPropertyListBinaryFormat_v1_0
								 options:0
								   error:nil];

		CFRelease(resolverDict);
	}

	return (NSData *)data;
}

- (NSData *)getDNSDataFromCurrentConfig:(dns_config_t *)dns_config
				 domain:(NSString *)domain
{
	if (dns_config == NULL || domain == nil) {
		SC_log(LOG_NOTICE, "Invalid dns_config/domain");
		return nil;
	}

	if ((dns_config->n_resolver > 0) && (dns_config->resolver != NULL)) {
		for (int i = 0; i < dns_config->n_resolver; i++) {
			dns_resolver_t	*	resolver;

			resolver = dns_config->resolver[i];
			if (resolver->domain != NULL &&
			    ![self isResolverMulticast:resolver]) {
				NSString	*	ns_domain_name;

				ns_domain_name = @(resolver->domain);
				if ([ns_domain_name isEqualToString:domain]) {
					return [self dataForResolver:resolver];
				} else {
					continue;
				}
			}
		}
	}

	return nil;
}

- (BOOL)isResolverMulticast:(dns_resolver_t *)resolver
{
	if (resolver->options == NULL) {
		return NO;
	}

	if (!strstr(resolver->options, "mdns")) {
		return NO;
	}

	return YES;
}

- (BOOL)isResolverPrivate:(dns_resolver_t *)resolver
{
	if (resolver->options == NULL) {
		return NO;
	}

	if (!strstr(resolver->options, "pdns")) {
		return NO;
	}

	return YES;
}

- (void)processSupplementalDNSResolvers:(dns_config_t *)dns_config
{
	NSMutableArray	*	deleteList;
	NSMutableArray	*	new_domain_list;
	NSCountedSet	*	duplicate_domain_list;
	NSMutableArray	*	old_domain_list;
	NSMutableArray	*	update_agent_list;


	deleteList = [NSMutableArray array];
	duplicate_domain_list = [[NSCountedSet alloc] initWithCapacity:0];
	new_domain_list = [NSMutableArray array];
	update_agent_list = [NSMutableArray array];
	old_domain_list = [self getAgentList:self.floatingDNSAgentList
				   agentType:kAgentTypeDNS
				agentSubType:kAgentSubTypeSupplemental];

	if (dns_config->resolver == NULL) {
		dns_config->n_resolver = 0;
	}
	if (dns_config->n_resolver > 0) {
		for (int i = 0; i < dns_config->n_resolver; i++) {
			dns_resolver_t	*	resolver;

			resolver = dns_config->resolver[i];
			if (resolver->domain != NULL &&
			    ![self isResolverPrivate:resolver] &&
			    ![self isResolverMulticast:resolver]) {
				NSString	*	ns_domain_name;

				ns_domain_name = [NSString stringWithCString:resolver->domain encoding:NSASCIIStringEncoding];
				[new_domain_list addObject:ns_domain_name];
			}
		}
	}

	[self cleanConflictingAgentsFromList:old_domain_list
				    new_list:new_domain_list
			     agentDictionary:self.floatingDNSAgentList];

	/* Sync between controller and current config */
	for (NSString *key in old_domain_list) {
		BOOL domain_present = NO;

		domain_present = [new_domain_list containsObject:key];
		if (domain_present == NO) {
			id agent;

			agent = [self.floatingDNSAgentList objectForKey:key];
			[self destroyFloatingAgent:agent];
		}
	}

	/*	At this point, whatever is in the controller's floating agent list,
		is present in the current DNS config. The current DNS config
		might have even more configs, not known to the controller, YET
	 */

	for (NSString *domain in old_domain_list) {
		id agent;
		id mapped_agent;

		agent = [self.floatingDNSAgentList objectForKey:domain];
		if (agent == nil) {
			continue;
		}

		/* Am I mapped to some agent? */
		mapped_agent = [agent getAgentMapping];
		if (mapped_agent) {
			/*	OK, this agent is mapped to some other agent. We compare this agent's data
			 *	to the current data of the agent to which it is mapped. If different, we destroy
			 *	the agent and later map it to someone else OR spawn a new one.
			 */
			NSData *mapped_agent_data;

			mapped_agent_data = [self getDNSDataFromCurrentConfig:dns_config domain:[mapped_agent getAssociatedEntity]];
			if (mapped_agent_data == nil || ![[agent getAgentData] isEqual:mapped_agent_data]) {
				/* Something changed for mapped agent */
				[deleteList addObject:agent];
				continue;
			}
		} else {
			/*	Since this agent is NOT mapped to any other agent, this agent is
			 *	registered with the kernel. So instead of destroying the agent and
			 *	re-registering it, just update it here.
			 *
			 *	All the agents which were mapped to this agent, will be deleted and
			 *	re-mapped, if the data changed.
			 */
			NSData *agent_data;

			agent_data = [self getDNSDataFromCurrentConfig:dns_config domain:[agent getAssociatedEntity]];
			if (![[agent getAgentData] isEqual:agent_data]) {
				/* Something changed for agent */
				[agent updateAgentData:agent_data];

				/*	The reason I don't publish the data to agent here is that, if there were
				 *	some agents mapping to this one, they will momentarily have a policy for
				 *	using this agent UUID for some domain based on this agent's previous data.
				 */
				[update_agent_list addObject:agent];

			}
		}
		[new_domain_list removeObject:domain];
	}

	for (id agent in deleteList) {
		SC_log(LOG_INFO, "Destroying agent %@ because something changed!", [agent getAgentName]);
		[self destroyFloatingAgent:agent];
	}

	for (id agent in update_agent_list) {
		[self publishToAgent:agent];
	}

	for (int idx = 0; idx < dns_config->n_resolver; idx++) {
		dns_resolver_t	*	resolver;

		resolver = dns_config->resolver[idx];
		if (resolver->domain != NULL &&
		    ![self isResolverPrivate:resolver] &&
		    ![self isResolverMulticast:resolver]) {
			NSData		*	data;
			NSUInteger		found;
			id			mapped_agent;
			NSString	*	ns_domain_name;

			ns_domain_name = @(resolver->domain);
			found = [new_domain_list indexOfObject:ns_domain_name];
			if (found == NSNotFound) {
				/* Nothing changed for this agent */
				continue;
			}

			/*	We will only process agents which are mapped AND if the agent they were mapped to, changed OR
			 *	agents for domains which we did not know before.
			 */

			 NSUInteger domainInstance = [duplicate_domain_list countForObject:ns_domain_name];
			 if (domainInstance > 0) {
				 /* domainInstance will be > 0, only if we have conflicting domains */
				 domainInstance++;
				 data = [self dataForResolver:resolver];

				 NSString *ns_domain_name_copy = [NSString stringWithFormat:@"%@" multipleEntitySuffix "%lu", ns_domain_name, (unsigned long)domainInstance];

				 BOOL ok = [self spawnFloatingAgent:[DNSAgent class]
							entity:ns_domain_name_copy
							agentSubType:kAgentSubTypeSupplemental
							addPolicyOfType:NEPolicyConditionTypeDomain
							publishData:data];
				 if (ok) {
					 id agent = [self.floatingDNSAgentList objectForKey:ns_domain_name_copy];
					 SC_log(LOG_INFO, "Duplicate DNS agent %@", [agent getAgentName]);;
				 }
			 } else {
				data = [self dataForResolver:resolver];
				mapped_agent = [self getAgentWithSameDataAndSubType:self.floatingDNSAgentList
										   data:data
										subType:kAgentSubTypeSupplemental];
				if (mapped_agent != nil) {
					[self spawnMappedFloatingAgent:mapped_agent
							entity:ns_domain_name
							agentSubType:kAgentSubTypeSupplemental
							addPolicyOfType:NEPolicyConditionTypeDomain
							updateData:data];
				} else {
					[self spawnFloatingAgent:[DNSAgent class]
							entity:ns_domain_name
							agentSubType:kAgentSubTypeSupplemental
							addPolicyOfType:NEPolicyConditionTypeDomain
							publishData:data];
				}
			 }

			[new_domain_list removeObjectAtIndex:found];
			[duplicate_domain_list addObject:ns_domain_name];
		}
	}

	return;

}

- (void)processDNSResolvers:(dns_config_t *)dns_config
{
	resolver_list_t *resolvers = [self copyResolverList:dns_config];
	if (resolvers) {
		/* Process Default resolvers */
		NSMutableArray *old_default_resolver_list = [self getAgentList:self.floatingDNSAgentList
								     agentType:kAgentTypeDNS
								  agentSubType:kAgentSubTypeDefault];

		// For default resolvers, their name will be '_defaultDNS', '_defaultDNS #2' so on...
		if (resolvers->n_default_resolvers > 0 && resolvers->default_resolvers != NULL) {
			for (uint32_t i = 0; i < resolvers->n_default_resolvers; i++) {
				dns_resolver_t *default_resolver = resolvers->default_resolvers[i];
				NSData	*	data;
				id		dnsAgent;
				NSString *	resolverName;

				data = [self dataForResolver:default_resolver];
				if (i == 0) {
					resolverName = @(dnsAgentDefault);
				} else {
					resolverName = [NSString stringWithFormat:@dnsAgentDefault multipleEntitySuffix "%d", i+1 ];
				}

				dnsAgent = [self.floatingDNSAgentList objectForKey:resolverName];

				if (dnsAgent != nil) {
					[old_default_resolver_list removeObject:resolverName];
					if ([data isEqual:[dnsAgent getAgentData]]) {
						/* Leave this agent in place. Nothing changed! */
						continue;
					} else {
						[self destroyFloatingAgent:dnsAgent];
					}
				}

				[self spawnFloatingAgent:[DNSAgent class]
						entity:resolverName
						agentSubType:kAgentSubTypeDefault
						addPolicyOfType:NEPolicyConditionTypeNone
						publishData:data];
			}
		}

		// Only agents that are NOT present in the new config, will be present in the list
		// and they need to be destroyed.
		[self deleteAgentList:self.floatingDNSAgentList list:old_default_resolver_list];

		/* Process Multicast resolvers */

		NSMutableArray *old_multicast_resolver_list = [self getAgentList:self.floatingDNSAgentList
								       agentType:kAgentTypeDNS
								    agentSubType:kAgentSubTypeMulticast];

		if (resolvers->n_multicast_resolvers > 0 && resolvers->multicast_resolvers != NULL) {
			for (uint32_t i = 0; i < resolvers->n_multicast_resolvers; i++) {
				dns_resolver_t * multicast_resolver = resolvers->multicast_resolvers[i];
				id		 dnsAgent;
				NSString *	 resolverName;

				if (multicast_resolver == NULL) {
					continue;
				}

				if (multicast_resolver->domain == NULL) {
					/* Multicast resolvers MUST have a domain */
					continue;
				}

				resolverName = @(multicast_resolver->domain);
				if (resolverName == NULL) {
					/* Multicast resolvers MUST have a domain */
					continue;
				}

				dnsAgent = [self.floatingDNSAgentList objectForKey:resolverName];
				if (dnsAgent != nil) {
					[old_multicast_resolver_list removeObject:resolverName];
					continue;
				}

				[self spawnFloatingAgent:[DNSAgent class]
						entity:resolverName
						agentSubType:kAgentSubTypeMulticast
						addPolicyOfType:NEPolicyConditionTypeDomain
						publishData:nil];
				// Don't care about data for mdns resolvers. Do we?
			}
		}

		[self deleteAgentList:self.floatingDNSAgentList list:old_multicast_resolver_list];

		/* Process Private resolvers */

		NSMutableArray *old_private_resolver_list = [self getAgentList:self.floatingDNSAgentList
								     agentType:kAgentTypeDNS
								  agentSubType:kAgentSubTypePrivate];

		if (resolvers->n_private_resolvers > 0 && resolvers->private_resolvers != NULL) {
			for (uint32_t i = 0; i < resolvers->n_private_resolvers; i++) {
				dns_resolver_t * private_resolver = resolvers->private_resolvers[i];
				id		 dnsAgent;
				NSString *	 resolverName;

				if (private_resolver == NULL) {
					continue;
				}

				if (private_resolver->domain == NULL) {
					/* private resolvers MUST have a domain */
					continue;
				}

				resolverName = @(private_resolver->domain);
				if (resolverName == nil) {
					/* Private resolvers MUST have a domain */
					continue;
				}

				dnsAgent = [self.floatingDNSAgentList objectForKey:resolverName];
				if (dnsAgent != nil) {
					[old_private_resolver_list removeObject:resolverName];
					continue;
				}

				[self spawnFloatingAgent:[DNSAgent class]
						entity:resolverName
						agentSubType:kAgentSubTypePrivate
						addPolicyOfType:NEPolicyConditionTypeDomain
						publishData:nil];
				// Don't care about data for pdns resolvers. Do we?
			}
		}

		[self deleteAgentList:self.floatingDNSAgentList list:old_private_resolver_list];
	}

	[self freeResolverList:resolvers];
}

- (void)processScopedDNSResolvers:(dns_config_t *)dns_config
{
	NSMutableArray	*	old_intf_list;
	old_intf_list = [self getAgentList:self.floatingDNSAgentList
				 agentType:kAgentTypeDNS
			      agentSubType:kAgentSubTypeScoped];

	if ((dns_config->n_scoped_resolver > 0) && (dns_config->scoped_resolver != NULL)) {
		for (int i = 0; i < dns_config->n_scoped_resolver; i++) {
			char			buf[IFNAMSIZ];
			NSData		*	data;
			id			dnsAgent;
			NSUInteger		idx;
			const char	*	if_name;
			NSString	*	ns_if_name;
			NSString	*	ns_if_name_with_prefix;
			dns_resolver_t	*	resolver;

			resolver = dns_config->scoped_resolver[i];
			if_name = my_if_indextoname(resolver->if_index, buf);
			if (if_name) {
				ns_if_name = @(if_name);
				ns_if_name_with_prefix = [NSString stringWithFormat:@"%s%@", prefixForInterfaceName, ns_if_name];
			} else {
				continue;
			}

			data = [self dataForResolver:resolver];
			idx = [old_intf_list indexOfObject:ns_if_name_with_prefix];

			if (idx == NSNotFound) {
				/* We need to spawn an agent */
				[self spawnFloatingAgent:[DNSAgent class]
						entity:ns_if_name_with_prefix
						agentSubType:kAgentSubTypeScoped
						addPolicyOfType:NEPolicyConditionTypeScopedInterface
						publishData:data];
				continue;
			} else {
				/* We have an agent on this interface. Update it */
				[old_intf_list removeObjectAtIndex:idx];
			}

			/* Get the DNS agent for this interface? */
			dnsAgent = [self.floatingDNSAgentList objectForKey:ns_if_name_with_prefix];
			if (dnsAgent != nil) {
				/* Do we need to update this agent? */
				[dnsAgent updateAgentData:data];
				if ([dnsAgent shouldUpdateAgent]) {
					[self publishToAgent:dnsAgent];
				}
			}
		}
	}

	[self deleteAgentList:self.floatingDNSAgentList list:old_intf_list];
}

- (void)processServiceSpecificDNSResolvers:(dns_config_t *)dns_config
{
	NSMutableArray	*	old_service_list;
	old_service_list = [self getAgentList:self.floatingDNSAgentList
				    agentType:kAgentTypeDNS
				 agentSubType:kAgentSubTypeServiceSpecific];

	if ((dns_config->n_service_specific_resolver > 0) && (dns_config->service_specific_resolver != NULL)) {
		for (int i = 0; i < dns_config->n_service_specific_resolver; i++) {
			NSData		*	data;
			id			dnsAgent;
			NSUInteger		idx;
			uint32_t		service_identifier;
			NSString	*	ns_service_identifier_with_prefix;
			dns_resolver_t	*	resolver;

			resolver = dns_config->service_specific_resolver[i];
			service_identifier = resolver->service_identifier;
			if (service_identifier != 0) {
				ns_service_identifier_with_prefix = [NSString stringWithFormat:@"%s%u", prefixForInterfaceName, service_identifier];
			} else {
				continue;
			}

			data = [self dataForResolver:resolver];
			idx = [old_service_list indexOfObject:ns_service_identifier_with_prefix];

			if (idx == NSNotFound) {
				/* We need to spawn an agent */
				[self spawnFloatingAgent:[DNSAgent class]
						  entity:ns_service_identifier_with_prefix
					    agentSubType:kAgentSubTypeServiceSpecific
					 addPolicyOfType:(POLICY_TYPE_NO_POLICY) /* Don't install a policy */
					     publishData:data];
				continue;
			} else {
				/* We have an agent on this interface. Update it */
				[old_service_list removeObjectAtIndex:idx];
			}

			/* Get the DNS agent for this interface? */
			dnsAgent = [self.floatingDNSAgentList objectForKey:ns_service_identifier_with_prefix];
			if (dnsAgent != nil) {
				/* Do we need to update this agent? */
				[dnsAgent updateAgentData:data];
				if ([dnsAgent shouldUpdateAgent]) {
					[self publishToAgent:dnsAgent];
				}
			}
		}
	}

	[self deleteAgentList:self.floatingDNSAgentList list:old_service_list];
}

#define ONION_RESOLVER_DOMAIN	"onion"
- (BOOL)isResolverOnion:(dns_resolver_t *)resolver
{
	if (resolver->domain != NULL &&
	    (strcmp(resolver->domain, ONION_RESOLVER_DOMAIN) == 0)) {
		return YES;
	}

	return NO;
}


- (void)processOnionResolver:(dns_config_t *)dns_config
{
	static NSUInteger policy_id = 0;

	if (dns_config == NULL) {
		goto remove_policy;
	}

	/* Run through the resolver configurations. We only care for the supplemental resolvers. */
	for (int32_t i = 0; i < dns_config->n_resolver; i++) {
		dns_resolver_t *resolver = dns_config->resolver[i];
		if ([self isResolverOnion:resolver]) {
			goto remove_policy;
		}
	}

	/* We do not have any such resolver. Add a system-wide "drop" policy for this domain */
	if (policy_id == 0) {
		NEPolicy *policy = [[NEPolicy alloc] initWithOrder:INIT_ORDER_FOR_DOMAIN_POLICY
							result:[NEPolicyResult drop]
							conditions:@[[NEPolicyCondition domain:@ONION_RESOLVER_DOMAIN]]];
		if (policy != nil) {
			policy_id = [self.policySession addPolicy:policy];
			if (![self.policySession apply]) {
				policy_id = 0;
				SC_log(LOG_NOTICE, "Could not add a [." ONION_RESOLVER_DOMAIN "] drop policy");
			} else {
				SC_log(LOG_INFO, "Added a [." ONION_RESOLVER_DOMAIN "] drop policy");
			}
		}
	}

	return;

remove_policy:

	/* We have such a resolver in the config OR no DNS config at all. Remove the system-wide "drop" policy for this domain */
	if (policy_id > 0) {
		[self.policySession removePolicyWithID:policy_id];
		if (![self.policySession apply]) {
			SC_log(LOG_NOTICE, "Could not remove the [." ONION_RESOLVER_DOMAIN "] drop policy");
		} else {
			policy_id = 0;
			SC_log(LOG_INFO, "Removed the [." ONION_RESOLVER_DOMAIN "] drop policy");
		}
	}

	return;
}
#undef ONION_RESOLVER_DOMAIN


- (void)processDNSChanges
{
	dns_config_t	*	dns_config;

	dns_config = dns_configuration_copy();
	if (dns_config == NULL) {
		SC_log(LOG_INFO, "No DNS configuration");
		NSMutableDictionary *copy = [self.floatingDNSAgentList copy];
		for (NSString *entity in copy) {
			id agent = [copy objectForKey:entity];

			[self destroyFloatingAgent:agent];
		}
		goto done;
	}

	[self processDNSResolvers:dns_config];
	[self processScopedDNSResolvers:dns_config];
	[self processSupplementalDNSResolvers:dns_config];
	[self processServiceSpecificDNSResolvers:dns_config];

done:

	[self processOnionResolver:dns_config];
	if (dns_config != NULL) {
		dns_configuration_free(dns_config);
	}
}

#pragma mark Helper functions

- (const void *)copyConfigAgentData:(NSMutableDictionary *)controllerDict
			       uuid:(uuid_t)requested_uuid
			     length:(uint64_t *)length
{
	if (length == NULL) {
		SC_log(LOG_NOTICE, "Invalid parameters for copying agent data");
		return NULL;
	}

	id agent = nil;
	void *buffer = NULL;
	*length = 0;

	for (NSString *key in controllerDict) {
		id temp_agent = [controllerDict objectForKey:key];

		uuid_t agent_uuid;

		[[temp_agent getAgentUUID] getUUIDBytes:agent_uuid];
		if (uuid_compare(agent_uuid, requested_uuid) == 0) {
			agent = temp_agent;
			break;
		}
	}

	if (agent == nil) {
		uuid_string_t uuid_str;
		uuid_unparse(requested_uuid, uuid_str);
		SC_log(LOG_NOTICE, "Invalid config agent uuid %s specified", uuid_str);
		return NULL;
	}

	NSData *data = [agent getAgentData];
	uint64_t len = [data length];
	if (len > 0) {
		*length = len;
		buffer = malloc((size_t)len);
		memcpy(buffer, [data bytes], len);
	}

	return (const void *)buffer;
}

- (const void *)copyProxyAgentData:(uuid_t)requested_uuid
			    length:(uint64_t *)length
{
	return [self copyConfigAgentData:self.floatingProxyAgentList
				    uuid:requested_uuid
				  length:length];
}

- (const void *)copyDNSAgentData:(uuid_t)requested_uuid
			  length:(uint64_t *)length
{
	return [self copyConfigAgentData:self.floatingDNSAgentList
				    uuid:requested_uuid
				  length:length];
}

- (NSData *)dataLengthSanityCheck:(id)agent
{
	NSData * data = [agent getAgentData];

	if ([data length] > CONFIG_AGENT_DATA_LIMIT) {
		/*  We impose a limit on the config agent data as 1KB.
		 *  If we have a data blob larger than this limit, do NOT publish it into the agent.
		 *  Instead publish a key which will trigger fetching of the configuration directly
		 *  through NWI server.
		 */
		NSMutableDictionary *data_dict = [NSMutableDictionary dictionary];

		NSUUID *uuid = [agent getAgentUUID];
		uuid_t c_uuid;
		[uuid getUUIDBytes:c_uuid];
		NSData *uuid_data = [[NSData alloc] initWithBytes:c_uuid length:sizeof(c_uuid)];
		[data_dict setValue:uuid_data forKey:@kConfigAgentOutOfBandDataUUID];

		NSData *new_data = [NSPropertyListSerialization dataWithPropertyList:data_dict
									      format:NSPropertyListBinaryFormat_v1_0
									     options:0
									       error:nil];

		return new_data;
	}

	return nil;
}

/*
 *	For conflicting agents, the convention is that its name & entity,
 *	will have a suffix " #<number>". This function will sanitize the
 *	suffix and just return the entity name
 */
- (NSString *)sanitizeEntity:(NSString *)entity
{
	NSRange range = [entity rangeOfString:@multipleEntitySuffix];
	if (range.location != NSNotFound) {
		NSString *str = [entity substringToIndex:range.location];
		return str;
	}

	return entity;
}

/*
 *	For interface names, there is a prefix to differentiate then
 *	from the domain name (iff there were conflicting domain names).
 *	Returns the sanitized interface name
 */
- (NSString *)sanitizeInterfaceName:(NSString *)intf
{
	NSRange range = [intf rangeOfString:@prefixForInterfaceName];
	if (range.location != NSNotFound) {
		NSString *str = [intf substringFromIndex:(range.location + strlen(prefixForInterfaceName))];
		return str;
	}

	return intf;
}

/*
 *	For conflicting agents, the convention is that its name & entity,
 *	will have a suffix " #<number>". This function will return that <number>
 */
- (int)entityInstanceNumber:(NSString *)entity
{
	NSRange range = [entity rangeOfString:@multipleEntitySuffix];
	if (range.location != NSNotFound) {
		NSString *str = [entity substringFromIndex:(range.location + strlen(multipleEntitySuffix))];
		return str.intValue;
	}

	return 0;
}

/*
 *	In case that we have conflicting DNS/Proxy domains
 *	This function will remove all those conflicting agents,
 *	so that we can start afresh with the new config
 */
- (void)cleanConflictingAgentsFromList:(NSMutableArray *)old_list
			      new_list:(NSMutableArray *)new_list
		       agentDictionary:(NSMutableDictionary *)agent_list
{
	NSCountedSet	*	duplicate_domain_list;

	for (NSString *domain in old_list) {
		/* If we had conflicting domains before, remove all of them */
		NSString *sanitizedDomain = [self sanitizeEntity:domain];
		if (![sanitizedDomain isEqualToString:domain]) {
			/* Destroy the original domain */
			id agent = [agent_list objectForKey:sanitizedDomain];
			[self destroyFloatingAgent:agent];

			/* Destroy the conflicting domain */
			agent = [agent_list objectForKey:domain];
			[self destroyFloatingAgent:agent];

			SC_log(LOG_INFO, "Removing conflicting domain: %@, %@", sanitizedDomain, domain);
		}
	}

	duplicate_domain_list = [[NSCountedSet alloc] initWithArray:new_list];
	for (NSString *domain in old_list) {
		if ([duplicate_domain_list countForObject:domain] > 1) {
			id agent = [agent_list objectForKey:domain];
			[self destroyFloatingAgent:agent];
			SC_log(LOG_INFO, "Removing domain %@ as it has duplicates in the current config", domain);
		}
	}
}

/*
 *	Get the list of agents from a specific dictionary.
 *	The list of agents will only consist of the ones which
 *	match the agent type and sub-type
 */

- (NSMutableArray *)getAgentList:(NSMutableDictionary *)all_agents
		       agentType:(AgentType)type
		    agentSubType:(AgentSubType)subtype
{
	NSMutableArray *list = [NSMutableArray array];
	NSArray *agentObjects = [all_agents allValues];

	for (id agent in agentObjects) {
		if (([agent getAgentType] == type) &&
		    ([agent getAgentSubType] == subtype)) {

			[list addObject:[agent getAssociatedEntity]];
		}
	}

	return list;
}

/*
 *	Destroy all the agents are listed in "list"
 */

- (void)deleteAgentList:(NSMutableDictionary *)all_agents
		   list:(NSMutableArray *)list
{
	for (NSString *intf in list) {
		id agent;

		agent = [all_agents objectForKey:intf];
		[self destroyFloatingAgent:agent];
	}
}

/*
 *	In order to not duplicate agents with same content,
 *	we map an agent X to agent Y, when their content is the same.
 *
 *	This function tries to find that agent Y
 */

- (id)getAgentWithSameDataAndSubType:(NSMutableDictionary *)agentList
				data:(NSData *)data
			     subType:(AgentSubType)subtype
{
	for (NSString *key in agentList) {
		id agent = [agentList objectForKey:key];
		if ([[agent getAgentData] isEqual:data]) {
			/* Do not map to default agents */
			if ([agent getAgentSubType] != subtype) {
				continue;
			}

			/* Return only registered agents */
			if ([agent getRegistrationObject] != nil) {
				return agent;
			}
		}
	}

	return nil;
}

#pragma mark Policy installation function

/*
 *	Add NECP policies for an agent
 */
- (BOOL)addPolicyToFloatingAgent:(id)agent
			  domain:(NSString *)domain
		  agentUUIDToUse:(NSUUID *)uuid
		      policyType:(NEPolicyConditionType)policyType
	 useControlPolicySession:(BOOL)useControlPolicySession
{
	NEPolicyCondition	*	condition = nil;
	NEPolicySession		*	session;
	uint32_t			multiple_entity_offset;
	NEPolicy		*	newPolicy;
	BOOL				ok;
	uint32_t			order;
	uint32_t			orderForSkip;
	NSMutableArray		*	policyArray;
	NSUInteger			policyID1;
	NSUInteger			policyID2;
	NEPolicyResult		*	result;
	uint32_t			skipOrder;
	AgentType			type;
	uint32_t			typeOffset;

	type = [agent getAgentType];
	typeOffset = (type == kAgentTypeDNS) ? 0 : 5000;
	skipOrder = (type == kAgentTypeDNS) ? 5000 : 0;

	multiple_entity_offset = (uint32_t)[self entityInstanceNumber:domain];
	domain = [self sanitizeEntity:domain];

	switch (policyType) {
		case NEPolicyConditionTypeScopedInterface:
			order = INIT_ORDER_FOR_SCOPED_INTERFACE_POLICY + typeOffset + multiple_entity_offset;
			domain = [self sanitizeInterfaceName:domain];
			condition = [NEPolicyCondition scopedInterface:domain];
			orderForSkip = SKIP_ORDER_FOR_SCOPED_INTERFACE_POLICY + typeOffset;
			break;

		case NEPolicyConditionTypeDomain:
			order = INIT_ORDER_FOR_DOMAIN_POLICY + typeOffset + multiple_entity_offset;
			condition = [NEPolicyCondition domain:domain];
			orderForSkip = SKIP_ORDER_FOR_DOMAIN_POLICY + typeOffset;
			break;

		case NEPolicyConditionTypeAllInterfaces:
			order = INIT_ORDER_FOR_DEFAULT_POLICY + typeOffset + multiple_entity_offset;
			condition = [NEPolicyCondition allInterfaces];
			orderForSkip = SKIP_ORDER_FOR_DEFAULT_POLICY + typeOffset;
			break;

		case NEPolicyConditionTypeNone:
			order = INIT_ORDER_FOR_DEFAULT_POLICY + typeOffset + multiple_entity_offset;
			orderForSkip = SKIP_ORDER_FOR_DEFAULT_POLICY + typeOffset;
			break;

		default:
			SC_log(LOG_NOTICE, "Invalid policy condition specified");
			return NO;
	}

	result = [NEPolicyResult netAgentUUID:uuid];
	newPolicy = [[NEPolicy alloc] initWithOrder:order
					     result:result
					 conditions: (condition ? @[condition] : nil)];

	if (newPolicy == nil) {
		SC_log(LOG_NOTICE, "Could not create a policy for agent %@", [agent getAgentName]);
		return NO;
	}

	if (useControlPolicySession) {
		if (self.controlPolicySession == nil) {
			/*	The NE policy session at "control" level for the controller */
			self.controlPolicySession = [self createPolicySession];
			if (self.controlPolicySession == nil) {
				SC_log(LOG_NOTICE, "Could not create a control policy session for agent %@", [agent getAgentName]);
				return NO;
			}
			[self.controlPolicySession setPriority:NEPolicySessionPriorityControl];
		}
		((ConfigAgent *)agent).preferredPolicySession = self.controlPolicySession;
	} else {
		((ConfigAgent *)agent).preferredPolicySession = self.policySession;
	}

	session = ((ConfigAgent *)agent).preferredPolicySession;

	policyID1 = [session addPolicy:newPolicy];
	if (policyID1 == 0) {
		SC_log(LOG_NOTICE, "Could not add a netagent policy for agent %@", [agent getAgentName]);
		return NO;
	}

	result = [NEPolicyResult skipWithOrder:skipOrder];
	newPolicy = [[NEPolicy alloc] initWithOrder:orderForSkip
					     result:result
					 conditions:(condition ? @[condition] : nil)];

	if (newPolicy == nil) {
		SC_log(LOG_NOTICE, "Could not create a policy for agent %@", [agent getAgentName]);
		return NO;
	}

	policyID2 = [session addPolicy:newPolicy];
	if (policyID2 == 0) {
		SC_log(LOG_NOTICE, "Could not add a skip policy for agent %@", [agent getAgentName]);
		return NO;
	}

	ok = [session apply];
	if (!ok) {
		SC_log(LOG_NOTICE, "Could not apply policy for agent %@", [agent getAgentName]);
		return NO;
	}

	policyArray = [self.policyDB objectForKey:[agent getAgentName]];
	if (policyArray == nil) {
		policyArray = [NSMutableArray array];
	}

	[policyArray addObject:numberToNSNumber(policyID1)];
	[policyArray addObject:numberToNSNumber(policyID2)];
	[self.policyDB setObject:policyArray forKey:[agent getAgentName]];

	return ok;
}

#pragma mark Agent manipulation functions

/*
 *	Create an agent
 */
- (BOOL)spawnFloatingAgent:(Class)agentClass
		entity:(NSString *)entity
		agentSubType:(AgentSubType)subtype
		addPolicyOfType:(NEPolicyConditionType)policyType
		publishData:(NSData *)data
{
	id	agent;
	BOOL	ok;
	NSMutableDictionary *	parameters;

	parameters =[NSMutableDictionary dictionary];
	[parameters setValue:entity forKey:@kEntityName];
	[parameters setValue:numberToNSNumber(subtype) forKey:@kAgentSubType];

	agent = [[agentClass alloc] initWithParameters:parameters];
	ok = [self registerAgent:agent];
	if (!ok) {
		return NO;
	}

	if (data) {
		/* Since we just spawned this agent, update its data */
		[agent updateAgentData:data];
		[self publishToAgent:agent];
	}

	/* Add a policy if there is a valid type. If POLICY_TYPE_NO_POLICY, then ignore policies.
	 * POLICY_TYPE_NO_POLICY will be set for service-specific agents, in which case we rely on
	 * service owners to install custom policies to point at the agents. */
	if (policyType >= NEPolicyConditionTypeNone) {
		BOOL useControlPolicySession = NO;
		if (subtype == kAgentSubTypeGlobal) {
			/* Policies for a Global scoped agents are at "control" level */
			useControlPolicySession = YES;
		}

		ok = [self addPolicyToFloatingAgent:agent
					     domain:entity
				     agentUUIDToUse:[agent agentUUID]
					 policyType:policyType
				useControlPolicySession:useControlPolicySession];

		if (!ok) {
			[self unregisterAgent:agent];
			return NO;
		}
	}

	SC_log(LOG_INFO, "Spawning floating agent for %@", entity);

	if ([agent getAgentType] == kAgentTypeProxy) {
		[self.floatingProxyAgentList setObject:agent forKey:entity];
	} else {
		[self.floatingDNSAgentList setObject:agent forKey:entity];
	}

	return ok;
}

/*
 *	Create an agent mapped to another agent.
 */
- (BOOL)spawnMappedFloatingAgent:(id)mapped_agent
			entity:(NSString *)entity
			agentSubType:(AgentSubType)subtype
			addPolicyOfType:(NEPolicyConditionType)policyType
			updateData:(NSData *)data
{
	id dummyAgent;
	NSMutableDictionary * parameters;

	parameters = [NSMutableDictionary dictionary];
	[parameters setValue:entity forKey:@kEntityName];
	[parameters setValue:numberToNSNumber(subtype) forKey:@kAgentSubType];

	dummyAgent = [[[mapped_agent class] alloc] initWithParameters:parameters];

	if (data) {
		/* Since we just spawned this agent, update its data.
		 * We do not publish it since this agent is mapped
		 * to an agent which already has the same data
		 */
		[dummyAgent updateAgentData:data];
	}

	BOOL useControlPolicySession = NO;
	if (subtype == kAgentSubTypeGlobal) {
		/* Policies for a Global scoped agents are at "control" level */
		useControlPolicySession = YES;
	}

	BOOL ok = [self addPolicyToFloatingAgent:dummyAgent
					domain:entity
					agentUUIDToUse:[mapped_agent agentUUID]
					policyType:policyType
					useControlPolicySession:useControlPolicySession];

	if (!ok) {
		return NO;
	}

	if ([mapped_agent getAgentType] == kAgentTypeProxy) {
		[self.floatingProxyAgentList setObject:dummyAgent forKey:entity];
	} else {
		[self.floatingDNSAgentList setObject:dummyAgent forKey:entity];
	}

	[dummyAgent setAgentMapping:mapped_agent];

	SC_log(LOG_INFO, "Mapped floating agent %@ to %@", [dummyAgent getAgentName], [mapped_agent getAgentName]);
	return YES;
}

/*
 *	Write into an agent
 */
- (BOOL)publishToAgent:(id)agent
{
	/* Before any data goes into the kernel, do a sanity check. */
	NSData *sanityCheckData = [self dataLengthSanityCheck:agent];
	NSData *tempAgentData = nil;

	if (sanityCheckData != nil) {
		/*  Data length is more than the limit! for updateNetworkAgent, the data blob
		 *  has to be a part of the agent object. Thus the temporary data replacement!
		 */
		tempAgentData = [[agent getAgentData] copy];
		[agent updateAgentData:sanityCheckData];
		SC_log(LOG_NOTICE, "Data too large for %@ (%lu bytes)!", [agent getAgentName], (unsigned long)[tempAgentData length]);
	}

	BOOL ok = NO;

	NWNetworkAgentRegistration *regObject = [agent valueForKey:@"registrationObject"];
	if (regObject != nil) {
		SC_log(LOG_NOTICE, "Publishing data to agent %@ (%lu bytes)", [agent getAgentName], (unsigned long)[[agent getAgentData] length]);
		ok = [regObject updateNetworkAgent:agent];
		if (!ok) {
			SC_log(LOG_NOTICE, "Could not update config agent");
		}
	} else {
		SC_log(LOG_NOTICE, "Config Agent not registered. Cannot Update");
	}

	if (tempAgentData != nil) {
		[agent updateAgentData:tempAgentData];
	}

	return ok;
}

/*
 *	Destroy an agent
 */
- (BOOL)destroyFloatingAgent:(id)agent
{
	BOOL ok = NO;

	if ( agent != nil) {
		NSMutableArray	*	policyArray;

		policyArray = [self.policyDB objectForKey:[agent getAgentName]];
		if (policyArray != nil) {
			NEPolicySession *	session = ((ConfigAgent *)agent).preferredPolicySession;
			BOOL 			result = NO;

			for (NSNumber *policyID in policyArray) {
				NSUInteger idVal;

				idVal = [policyID unsignedIntegerValue];
				result = [session removePolicyWithID:idVal];
				if (result == NO) {
					SC_log(LOG_NOTICE, "Could not remove policy %@ for agent %@", [session policyWithID:idVal], [agent getAgentName]);
				}
			}

			result = [session apply];
			if (result == NO) {
				SC_log(LOG_NOTICE, "Could not apply removed policies for agent %@", [agent getAgentName]);
			}

			[self.policyDB removeObjectForKey:[agent getAgentName]];
		}

		if ([agent getAgentType] == kAgentTypeProxy) {
			[self.floatingProxyAgentList removeObjectForKey:[agent getAssociatedEntity]];
		} else {
			[self.floatingDNSAgentList removeObjectForKey:[agent getAssociatedEntity]];
		}

		if ([agent getRegistrationObject] != nil) {
			[self unregisterAgent:agent];
		}

		SC_log(LOG_INFO, "X - Destroyed agent %@", [agent getAgentName]);

		/* Check if we need to close the "control" policy session */
		if (self.controlPolicySession != nil) {
			NSMutableArray *globalProxyAgentList;
			NSMutableArray *globalDNSAgentList;
			globalProxyAgentList = [self getAgentList:self.floatingProxyAgentList agentType:kAgentTypeProxy agentSubType:kAgentSubTypeGlobal];
			globalDNSAgentList = [self getAgentList:self.floatingDNSAgentList agentType:kAgentTypeDNS agentSubType:kAgentSubTypeGlobal];

			if ([globalProxyAgentList count] == 0 &&
			    [globalDNSAgentList count] == 0) {
				ok = [self.controlPolicySession removeAllPolicies];
				if (!ok) {
					SC_log(LOG_ERR, "Could not remove policies for agent %@", [agent getAgentName]);
				}

				ok = [self.controlPolicySession apply];
				if (!ok) {
					SC_log(LOG_ERR, "Could not apply policy change for agent %@", [agent getAgentName]);
				}

				self.controlPolicySession = nil;
				SC_log(LOG_NOTICE, "Closed control policy session");
			}
		}

		ok = YES;
	}

	return ok;
}

/*
 *	Register an agent
 */
- (BOOL)registerAgent:(id)agent
{
	BOOL ok = NO;

	NWNetworkAgentRegistration *registration = [[NWNetworkAgentRegistration alloc] initWithNetworkAgentClass:[agent class]];

	ok = [registration registerNetworkAgent:agent];
	if (!ok) {
		SC_log(LOG_NOTICE, "Could not register config agent");
		goto done;
	}

	[agent addAgentRegistrationObject:registration];

done:
	return ok;
}

/*
 *	Unregister an agent
 */
- (BOOL)unregisterAgent:(id)agent
{
	BOOL ok = false;

	NWNetworkAgentRegistration *regObject = [agent valueForKey:@"registrationObject"];
	if (regObject != nil) {
		ok = [regObject unregisterNetworkAgent];
		if (!ok) {
			SC_log(LOG_NOTICE, "Could not unregister config agent");
		}
	} else {
		SC_log(LOG_NOTICE, "Config Agent not registered. Cannot unregister");
	}

	return ok;
}


@end
