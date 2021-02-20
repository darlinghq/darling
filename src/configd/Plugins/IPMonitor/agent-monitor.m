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

#import "controller.h"

static Boolean
haveNetworkExtensionFramework()
{
	Boolean	haveFramework;

	haveFramework = ([NEPolicy class] != nil);
	return haveFramework;
}

void
process_AgentMonitor(void)
{
	if (!haveNetworkExtensionFramework()) {
		return;
	}

	SC_log(LOG_DEBUG, "Triggering AgentMonitor");
	@autoreleasepool {
		AgentController *controller = [AgentController sharedController];
		if (controller == nil) {
			SC_log(LOG_ERR, "AgentController could not be initialized");
			return;
		}

		dispatch_sync(controller.controllerQueue, ^{
			[[AgentController sharedController] processDNSChanges];
			[[AgentController sharedController] processProxyChanges];
		});
	}

	return;
}

void
process_AgentMonitor_DNS(void)
{
	if (!haveNetworkExtensionFramework()) {
		return;
	}

	SC_log(LOG_DEBUG, "Triggering AgentMonitor for DNS");
	@autoreleasepool {
		AgentController *controller = [AgentController sharedController];
		if (controller == nil) {
			SC_log(LOG_ERR, "AgentController could not be initialized");
			return;
		}

		dispatch_sync(controller.controllerQueue, ^{
			[[AgentController sharedController] processDNSChanges];
		});
	}

	return;
}

void
process_AgentMonitor_Proxy(void)
{
	if (!haveNetworkExtensionFramework()) {
		return;
	}

	SC_log(LOG_DEBUG, "Triggering AgentMonitor for Proxy");
	@autoreleasepool {
		AgentController *controller = [AgentController sharedController];
		if (controller == nil) {
			SC_log(LOG_ERR, "AgentController could not be initialized");
			return;
		}

		dispatch_sync(controller.controllerQueue, ^{
			[[AgentController sharedController] processProxyChanges];
		});
	}

	return;
}

const void *
copy_proxy_information_for_agent_uuid(uuid_t uuid, uint64_t *length)
{
	__block const void *buffer = NULL;

	if (!haveNetworkExtensionFramework()) {
		return NULL;
	}

	@autoreleasepool {
		AgentController *controller = [AgentController sharedController];
		if (controller == nil) {
			SC_log(LOG_ERR, "AgentController could not be initialized");
			return NULL;
		}

		dispatch_sync(controller.controllerQueue, ^{
			buffer = [[AgentController sharedController] copyProxyAgentData:uuid
										 length:length];
		});
	}

	return buffer;
}

const void *
copy_dns_information_for_agent_uuid(uuid_t uuid, uint64_t *length)
{
	__block const void *buffer = NULL;

	if (!haveNetworkExtensionFramework()) {
		return NULL;
	}

	@autoreleasepool {
		AgentController *controller = [AgentController sharedController];
		if (controller == nil) {
			SC_log(LOG_ERR, "AgentController could not be initialized");
			return NULL;
		}

		dispatch_sync(controller.controllerQueue, ^{
			buffer = [[AgentController sharedController] copyDNSAgentData:uuid
									       length:length];
		});
	}

	return buffer;
}
