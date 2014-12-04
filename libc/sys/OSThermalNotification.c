/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#include <dispatch/dispatch.h>
#include <libkern/OSThermalNotification.h>
#include <notify.h>

#define OSThermalStatusName "com.apple.system.thermalstatus"

const char * const kOSThermalNotificationName = OSThermalStatusName; 

static const char * const kOSThermalMitigationNames[kOSThermalMitigationCount] = {
	OSThermalStatusName,
	"com.apple.system.thermalmitigation.70percenttorch",
	"com.apple.system.thermalmitigation.70percentbacklight",
	"com.apple.system.thermalmitigation.50percenttorch",
	"com.apple.system.thermalmitigation.50percentbacklight",
	"com.apple.system.thermalmitigation.disabletorch",
	"com.apple.system.thermalmitigation.25percentbacklight",
	"com.apple.system.thermalmitigation.disablemapshalo",
	"com.apple.system.thermalmitigation.appterminate",
	"com.apple.system.thermalmitigation.devicerestart",
	"com.apple.system.thermalmitigation.thermaltableready"
};

static int tokens[kOSThermalMitigationCount];
static dispatch_once_t predicates[kOSThermalMitigationCount];
static bool thermalLevelsReady = false;

OSThermalNotificationLevel _OSThermalNotificationLevelForBehavior(int behavior)
{
	uint64_t val = OSThermalNotificationLevelAny;
	if (behavior >= 0 && behavior < kOSThermalMitigationCount) {
		dispatch_once(&predicates[behavior], ^{
			(void)notify_register_check(kOSThermalMitigationNames[behavior], &tokens[behavior]);
		});
		(void)notify_get_state(tokens[behavior], &val);
	}
	return (OSThermalNotificationLevel)val;
}

void _OSThermalNotificationSetLevelForBehavior(int level, int behavior)
{
	uint64_t val = (uint64_t)level;
	if (behavior >= 0 && behavior < kOSThermalMitigationCount) {
		dispatch_once(&predicates[behavior], ^{
			(void)notify_register_check(kOSThermalMitigationNames[behavior], &tokens[behavior]);
		});
		(void)notify_set_state(tokens[behavior], val);

		// Note:  
		// - We are ready when we program in the appterminate value. 
		// - Assumes that user programs kOSThermalMitigationNone level less than 
		//   kOSThermalMitigationAppTerminate & kOSThermalMitigationDeviceRestart  
		if (behavior == kOSThermalMitigationAppTerminate) {
			dispatch_once(&predicates[kOSThermalMitigationThermalTableReady], ^{
				(void)notify_register_check(kOSThermalMitigationNames[kOSThermalMitigationThermalTableReady], &tokens[kOSThermalMitigationThermalTableReady]);
			});
			(void)notify_set_state(tokens[kOSThermalMitigationThermalTableReady], kOSThermalMitigationCount);
		}
	}
}


OSThermalNotificationLevel OSThermalNotificationCurrentLevel(void)
{
	if (thermalLevelsReady) {
		return _OSThermalNotificationLevelForBehavior(kOSThermalMitigationNone);
	}

	uint64_t tableReady = 0;

	dispatch_once(&predicates[kOSThermalMitigationThermalTableReady], ^{
		(void)notify_register_check(kOSThermalMitigationNames[kOSThermalMitigationThermalTableReady], &tokens[kOSThermalMitigationThermalTableReady]);
	});
	(void)notify_get_state(tokens[kOSThermalMitigationThermalTableReady], &tableReady);

	// If we are ready then optimize this so we don't call dispatch everytime.
	if (tableReady == kOSThermalMitigationCount) {
		thermalLevelsReady = true;
		return _OSThermalNotificationLevelForBehavior(kOSThermalMitigationNone); 
	}
	else {
		// Allow reset so we can dynamically change the table without thermal trap screen appearing. 
		thermalLevelsReady = false;
	}

	// Not ready returns -1, which should not be equal or greater than any other thermal state. 
	return OSThermalNotificationLevelAny;
}
