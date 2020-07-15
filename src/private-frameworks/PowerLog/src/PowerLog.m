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


#include <PowerLog/PowerLog.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* PLBatchDropMessages(void)
{
    if (verbose) puts("STUB: PLBatchDropMessages called");
    return NULL;
}

void* PLBatteryGaugeGetSnapshot(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeGetSnapshot called");
    return NULL;
}

void* PLBatteryGaugePauseTracingForBundleId(void)
{
    if (verbose) puts("STUB: PLBatteryGaugePauseTracingForBundleId called");
    return NULL;
}

void* PLBatteryGaugePauseTracingForPid(void)
{
    if (verbose) puts("STUB: PLBatteryGaugePauseTracingForPid called");
    return NULL;
}

void* PLBatteryGaugePauseTracingForProcessName(void)
{
    if (verbose) puts("STUB: PLBatteryGaugePauseTracingForProcessName called");
    return NULL;
}

void* PLBatteryGaugeResumeTracingForBundleId(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeResumeTracingForBundleId called");
    return NULL;
}

void* PLBatteryGaugeResumeTracingForPid(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeResumeTracingForPid called");
    return NULL;
}

void* PLBatteryGaugeResumeTracingForProcessName(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeResumeTracingForProcessName called");
    return NULL;
}

void* PLBatteryGaugeStartTracingForBundleId(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeStartTracingForBundleId called");
    return NULL;
}

void* PLBatteryGaugeStartTracingForPid(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeStartTracingForPid called");
    return NULL;
}

void* PLBatteryGaugeStartTracingForProcessName(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeStartTracingForProcessName called");
    return NULL;
}

void* PLBatteryGaugeStopTracingForBundleId(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeStopTracingForBundleId called");
    return NULL;
}

void* PLBatteryGaugeStopTracingForPid(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeStopTracingForPid called");
    return NULL;
}

void* PLBatteryGaugeStopTracingForProcessName(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeStopTracingForProcessName called");
    return NULL;
}

void* PLBatteryGaugeVersion(void)
{
    if (verbose) puts("STUB: PLBatteryGaugeVersion called");
    return NULL;
}

void* PLBatteryLifeMonitorLogCoreDuetAdmissionEvent(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorLogCoreDuetAdmissionEvent called");
    return NULL;
}

void* PLBatteryLifeMonitorLogCoreDuetEvent(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorLogCoreDuetEvent called");
    return NULL;
}

void* PLBatteryLifeMonitorLogCoreDuetEventsAggregate(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorLogCoreDuetEventsAggregate called");
    return NULL;
}

void* PLBatteryLifeMonitorLogCoreDuetPredictionEvents(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorLogCoreDuetPredictionEvents called");
    return NULL;
}

void* PLBatteryLifeMonitorLogDuetEvent(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorLogDuetEvent called");
    return NULL;
}

void* PLBatteryLifeMonitorReportBatteryCapacityForCoreDuet(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorReportBatteryCapacityForCoreDuet called");
    return NULL;
}

void* PLBatteryLifeMonitorReportCumulativeTotalDiscretionaryEnergyCoreDuet(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorReportCumulativeTotalDiscretionaryEnergyCoreDuet called");
    return NULL;
}

void* PLBatteryLifeMonitorReportDiscretionaryBudgetForDuet(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorReportDiscretionaryBudgetForDuet called");
    return NULL;
}

void* PLBatteryLifeMonitorReportEnergyCoreDuet(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorReportEnergyCoreDuet called");
    return NULL;
}

void* PLBatteryLifeMonitorReportEnergyDuet(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorReportEnergyDuet called");
    return NULL;
}

void* PLBatteryLifeMonitorStopAllDuetEvents(void)
{
    if (verbose) puts("STUB: PLBatteryLifeMonitorStopAllDuetEvents called");
    return NULL;
}

void* PLBatteryUsageUIQuery(void)
{
    if (verbose) puts("STUB: PLBatteryUsageUIQuery called");
    return NULL;
}

void* PLBuildEnergyBucket(void)
{
    if (verbose) puts("STUB: PLBuildEnergyBucket called");
    return NULL;
}

void* PLBuildEnergyBucketTillNow(void)
{
    if (verbose) puts("STUB: PLBuildEnergyBucketTillNow called");
    return NULL;
}

void* PLBuildUsageSnapshot(void)
{
    if (verbose) puts("STUB: PLBuildUsageSnapshot called");
    return NULL;
}

void* PLCopyPowerlog(void)
{
    if (verbose) puts("STUB: PLCopyPowerlog called");
    return NULL;
}

void* PLCrashMoverCopyPowerlog(void)
{
    if (verbose) puts("STUB: PLCrashMoverCopyPowerlog called");
    return NULL;
}

void* PLForceBatching(void)
{
    if (verbose) puts("STUB: PLForceBatching called");
    return NULL;
}

void* PLLogEvent(void)
{
    if (verbose) puts("STUB: PLLogEvent called");
    return NULL;
}

void* PLLogRegisteredAggregateEvent(void)
{
    if (verbose) puts("STUB: PLLogRegisteredAggregateEvent called");
    return NULL;
}

void* PLLogRegisteredEvent(void)
{
    if (verbose) puts("STUB: PLLogRegisteredEvent called");
    return NULL;
}

void* PLLogRegisteredEventFilterByInterval(void)
{
    if (verbose) puts("STUB: PLLogRegisteredEventFilterByInterval called");
    return NULL;
}

void* PLLogTimeSensitiveRegisteredEvent(void)
{
    if (verbose) puts("STUB: PLLogTimeSensitiveRegisteredEvent called");
    return NULL;
}

void* PLMovePowerlogsToCR(void)
{
    if (verbose) puts("STUB: PLMovePowerlogsToCR called");
    return NULL;
}

void* PLQuery(void)
{
    if (verbose) puts("STUB: PLQuery called");
    return NULL;
}

void* PLQueryRegistered(void)
{
    if (verbose) puts("STUB: PLQueryRegistered called");
    return NULL;
}

void* PLShouldLogEvent(void)
{
    if (verbose) puts("STUB: PLShouldLogEvent called");
    return NULL;
}

void* PLShouldLogRegisteredEvent(void)
{
    if (verbose) puts("STUB: PLShouldLogRegisteredEvent called");
    return NULL;
}

void* PLSysdiagnoseCopyPowerlog(void)
{
    if (verbose) puts("STUB: PLSysdiagnoseCopyPowerlog called");
    return NULL;
}

void* PLTalkToPowerlogHelper(void)
{
    if (verbose) puts("STUB: PLTalkToPowerlogHelper called");
    return NULL;
}
