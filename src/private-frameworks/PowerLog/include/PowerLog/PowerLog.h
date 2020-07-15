/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _PowerLog_H_
#define _PowerLog_H_

#import <Foundation/Foundation.h>

#import <PowerLog/PLCoreDuetEventInterval.h>
#import <PowerLog/PLCoreDuetEvent.h>
#import <PowerLog/PLEnergyBucket.h>
#import <PowerLog/PLUsageSnapshot.h>
#import <PowerLog/PLDuetActivitySchedulerUsageSnapshot.h>
#import <PowerLog/PLNetworkEnergyModel.h>
#import <PowerLog/PLModelingUtilities.h>
#import <PowerLog/PLClientLogAggregator.h>
#import <PowerLog/PLClientLogger.h>

void* PLBatchDropMessages(void);
void* PLBatteryGaugeGetSnapshot(void);
void* PLBatteryGaugePauseTracingForBundleId(void);
void* PLBatteryGaugePauseTracingForPid(void);
void* PLBatteryGaugePauseTracingForProcessName(void);
void* PLBatteryGaugeResumeTracingForBundleId(void);
void* PLBatteryGaugeResumeTracingForPid(void);
void* PLBatteryGaugeResumeTracingForProcessName(void);
void* PLBatteryGaugeStartTracingForBundleId(void);
void* PLBatteryGaugeStartTracingForPid(void);
void* PLBatteryGaugeStartTracingForProcessName(void);
void* PLBatteryGaugeStopTracingForBundleId(void);
void* PLBatteryGaugeStopTracingForPid(void);
void* PLBatteryGaugeStopTracingForProcessName(void);
void* PLBatteryGaugeVersion(void);
void* PLBatteryLifeMonitorLogCoreDuetAdmissionEvent(void);
void* PLBatteryLifeMonitorLogCoreDuetEvent(void);
void* PLBatteryLifeMonitorLogCoreDuetEventsAggregate(void);
void* PLBatteryLifeMonitorLogCoreDuetPredictionEvents(void);
void* PLBatteryLifeMonitorLogDuetEvent(void);
void* PLBatteryLifeMonitorReportBatteryCapacityForCoreDuet(void);
void* PLBatteryLifeMonitorReportCumulativeTotalDiscretionaryEnergyCoreDuet(void);
void* PLBatteryLifeMonitorReportDiscretionaryBudgetForDuet(void);
void* PLBatteryLifeMonitorReportEnergyCoreDuet(void);
void* PLBatteryLifeMonitorReportEnergyDuet(void);
void* PLBatteryLifeMonitorStopAllDuetEvents(void);
void* PLBatteryUsageUIQuery(void);
void* PLBuildEnergyBucket(void);
void* PLBuildEnergyBucketTillNow(void);
void* PLBuildUsageSnapshot(void);
void* PLCopyPowerlog(void);
void* PLCrashMoverCopyPowerlog(void);
void* PLForceBatching(void);
void* PLLogEvent(void);
void* PLLogRegisteredAggregateEvent(void);
void* PLLogRegisteredEventFilterByInterval(void);
void* PLMovePowerlogsToCR(void);
void* PLQuery(void);
void* PLQueryRegistered(void);
void* PLShouldLogEvent(void);
void* PLSysdiagnoseCopyPowerlog(void);
void* PLTalkToPowerlogHelper(void);

// NOTE(@facekapow):
// i'm not entirely sure where to put this constant, but this seems like as good a place as any
#define PLClientIDSecurity (0)

// the first parameter to these functions is the type of `PLClientID<whatever>`,
// but i'm not entirely sure what that is. `int` is a good guess
void* PLShouldLogRegisteredEvent(int client_id, CFStringRef event_name);
void* PLLogRegisteredEvent(int client_id, CFStringRef event_name, CFDictionaryRef event_dictionary, void* last_parameter);
void* PLLogTimeSensitiveRegisteredEvent(int client_id, CFStringRef event_name, CFDictionaryRef event_dictionary, void* last_parameter);

#endif
