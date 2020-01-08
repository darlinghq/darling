/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#include <CoreLocation/CoreLocation.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CLClientCreateIso6709Notation(void)
{
    if (verbose) puts("STUB: CLClientCreateIso6709Notation called");
    return NULL;
}

void* CLClientCreateWithBundleIdentifierAndPath(void)
{
    if (verbose) puts("STUB: CLClientCreateWithBundleIdentifierAndPath called");
    return NULL;
}

void* CLClientCreateWithBundleIdentifierAndPathOnRunLoop(void)
{
    if (verbose) puts("STUB: CLClientCreateWithBundleIdentifierAndPathOnRunLoop called");
    return NULL;
}

void* CLClientDisplayStats(void)
{
    if (verbose) puts("STUB: CLClientDisplayStats called");
    return NULL;
}

void* CLClientGetAuthorizationStatus(void)
{
    if (verbose) puts("STUB: CLClientGetAuthorizationStatus called");
    return NULL;
}

void* CLClientGetDistanceCoordinates(void)
{
    if (verbose) puts("STUB: CLClientGetDistanceCoordinates called");
    return NULL;
}

void* CLClientGetDistanceHighPrecision(void)
{
    if (verbose) puts("STUB: CLClientGetDistanceHighPrecision called");
    return NULL;
}

void* CLClientGetError(void)
{
    if (verbose) puts("STUB: CLClientGetError called");
    return NULL;
}

void* CLClientGetHeading(void)
{
    if (verbose) puts("STUB: CLClientGetHeading called");
    return NULL;
}

void* CLClientGetLocation(void)
{
    if (verbose) puts("STUB: CLClientGetLocation called");
    return NULL;
}

void* CLClientGetTypeID(void)
{
    if (verbose) puts("STUB: CLClientGetTypeID called");
    return NULL;
}

void* CLClientInvalidate(void)
{
    if (verbose) puts("STUB: CLClientInvalidate called");
    return NULL;
}

void* CLClientIsHeadingAvailable(void)
{
    if (verbose) puts("STUB: CLClientIsHeadingAvailable called");
    return NULL;
}

void* CLClientIsLocationServicesAvailable(void)
{
    if (verbose) puts("STUB: CLClientIsLocationServicesAvailable called");
    return NULL;
}

void* CLClientIsLocationServicesEnabled(void)
{
    if (verbose) puts("STUB: CLClientIsLocationServicesEnabled called");
    return NULL;
}

void* CLClientIsValid(void)
{
    if (verbose) puts("STUB: CLClientIsValid called");
    return NULL;
}

void* CLClientLog(void)
{
    if (verbose) puts("STUB: CLClientLog called");
    return NULL;
}

void* CLClientLogDump(void)
{
    if (verbose) puts("STUB: CLClientLogDump called");
    return NULL;
}

void* CLClientRequestAuthorization(void)
{
    if (verbose) puts("STUB: CLClientRequestAuthorization called");
    return NULL;
}

void* CLClientRetrieveAuthorizationStatus(void)
{
    if (verbose) puts("STUB: CLClientRetrieveAuthorizationStatus called");
    return NULL;
}

void* CLClientRetrieveLocation(void)
{
    if (verbose) puts("STUB: CLClientRetrieveLocation called");
    return NULL;
}

void* CLClientRetrieveLocationWithDynamicAccuracyReductionAndAlteredAccessoryLocations(void)
{
    if (verbose) puts("STUB: CLClientRetrieveLocationWithDynamicAccuracyReductionAndAlteredAccessoryLocations called");
    return NULL;
}

void* CLClientSetAllowsMapCorrection(void)
{
    if (verbose) puts("STUB: CLClientSetAllowsMapCorrection called");
    return NULL;
}

void* CLClientSetCallback(void)
{
    if (verbose) puts("STUB: CLClientSetCallback called");
    return NULL;
}

void* CLClientSetShowLocationPrompt(void)
{
    if (verbose) puts("STUB: CLClientSetShowLocationPrompt called");
    return NULL;
}

void* CLClientShowHeadingCalibration(void)
{
    if (verbose) puts("STUB: CLClientShowHeadingCalibration called");
    return NULL;
}

void* CLClientShutdownDaemon(void)
{
    if (verbose) puts("STUB: CLClientShutdownDaemon called");
    return NULL;
}

void* CLClientStartHeadingUpdates(void)
{
    if (verbose) puts("STUB: CLClientStartHeadingUpdates called");
    return NULL;
}

void* CLClientStartLocationUpdates(void)
{
    if (verbose) puts("STUB: CLClientStartLocationUpdates called");
    return NULL;
}

void* CLClientStartLocationUpdatesWithDynamicAccuracyReductionAndAlteredAccessoryLocations(void)
{
    if (verbose) puts("STUB: CLClientStartLocationUpdatesWithDynamicAccuracyReductionAndAlteredAccessoryLocations called");
    return NULL;
}

void* CLClientStartVehicleHeadingUpdates(void)
{
    if (verbose) puts("STUB: CLClientStartVehicleHeadingUpdates called");
    return NULL;
}

void* CLClientStartVehicleSpeedUpdates(void)
{
    if (verbose) puts("STUB: CLClientStartVehicleSpeedUpdates called");
    return NULL;
}

void* CLClientStopHeadingUpdates(void)
{
    if (verbose) puts("STUB: CLClientStopHeadingUpdates called");
    return NULL;
}

void* CLClientStopLocationUpdates(void)
{
    if (verbose) puts("STUB: CLClientStopLocationUpdates called");
    return NULL;
}

void* CLClientStopVehicleHeadingUpdates(void)
{
    if (verbose) puts("STUB: CLClientStopVehicleHeadingUpdates called");
    return NULL;
}

void* CLClientStopVehicleSpeedUpdates(void)
{
    if (verbose) puts("STUB: CLClientStopVehicleSpeedUpdates called");
    return NULL;
}

void* CLCopyAppsUsingLocation(void)
{
    if (verbose) puts("STUB: CLCopyAppsUsingLocation called");
    return NULL;
}

void* CLCopyTechnologiesInUse(void)
{
    if (verbose) puts("STUB: CLCopyTechnologiesInUse called");
    return NULL;
}

void* CLGenerateRoadSegmentId(void)
{
    if (verbose) puts("STUB: CLGenerateRoadSegmentId called");
    return NULL;
}

void* CLGetControlPlaneStatusReport(void)
{
    if (verbose) puts("STUB: CLGetControlPlaneStatusReport called");
    return NULL;
}

void* CLGetLocationDefault(void)
{
    if (verbose) puts("STUB: CLGetLocationDefault called");
    return NULL;
}

void* CLGetStatusBarIconState(void)
{
    if (verbose) puts("STUB: CLGetStatusBarIconState called");
    return NULL;
}

void* CLInternalPerformMigration(void)
{
    if (verbose) puts("STUB: CLInternalPerformMigration called");
    return NULL;
}

void* CLLocationCoordinate2DGetDistanceFrom(void)
{
    if (verbose) puts("STUB: CLLocationCoordinate2DGetDistanceFrom called");
    return NULL;
}

void* CLLocationCoordinate2DIsValid(void)
{
    if (verbose) puts("STUB: CLLocationCoordinate2DIsValid called");
    return NULL;
}

void* CLLocationCoordinate2DMake(void)
{
    if (verbose) puts("STUB: CLLocationCoordinate2DMake called");
    return NULL;
}

void* CLPingDaemon(void)
{
    if (verbose) puts("STUB: CLPingDaemon called");
    return NULL;
}

void* CLSetLocationDefault(void)
{
    if (verbose) puts("STUB: CLSetLocationDefault called");
    return NULL;
}

void* CLSetMapMatchingRouteHint(void)
{
    if (verbose) puts("STUB: CLSetMapMatchingRouteHint called");
    return NULL;
}

void* CLStartStopAdvertisingBeacon(void)
{
    if (verbose) puts("STUB: CLStartStopAdvertisingBeacon called");
    return NULL;
}
