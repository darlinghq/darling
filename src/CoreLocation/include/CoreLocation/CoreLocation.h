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


#ifndef _CoreLocation_H_
#define _CoreLocation_H_

#import <Foundation/Foundation.h>

#import <CoreLocation/CLHarvesterServerInterface.h>
#import <CoreLocation/CLLocationInternalServiceProtocol.h>
#import <CoreLocation/CLLocationManagerRoutineClientInterface.h>
#import <CoreLocation/CLLocationManagerRoutineServerInterface.h>
#import <CoreLocation/CLSimulationLocationProtocol.h>
#import <CoreLocation/CLSimulationXPCServerInterface.h>
#import <CoreLocation/_CLLocationManagerRoutineProxy.h>
#import <CoreLocation/CLLocationManagerRoutine.h>
#import <CoreLocation/CLEmergencyEnablementAssertion.h>
#import <CoreLocation/CLLocationManagerStateTracker.h>
#import <CoreLocation/CLLocationManagerInternal.h>
#import <CoreLocation/CLLocationManager.h>
#import <CoreLocation/CLHeadingInternal.h>
#import <CoreLocation/CLHeading.h>
#import <CoreLocation/CLRegionInternal.h>
#import <CoreLocation/CLRegion.h>
#import <CoreLocation/CLCircularRegion.h>
#import <CoreLocation/CLBeaconRegion.h>
#import <CoreLocation/CLBeaconInternal.h>
#import <CoreLocation/CLBeacon.h>
#import <CoreLocation/CLSimulationManager.h>
#import <CoreLocation/CLGeocoderInternal.h>
#import <CoreLocation/CLGeocoder.h>
#import <CoreLocation/CLLocationInternalClient.h>
#import <CoreLocation/CLPlacemarkInternal.h>
#import <CoreLocation/CLPlacemark.h>
#import <CoreLocation/CLHarvester.h>
#import <CoreLocation/CLStateTracker.h>
#import <CoreLocation/CLLocationInternal.h>
#import <CoreLocation/CLLocation.h>
#import <CoreLocation/CLAssertion.h>
#import <CoreLocation/CLTilesManagerClientInternal.h>
#import <CoreLocation/CLTilesManagerClient.h>
#import <CoreLocation/_CLRangingPeerInternal.h>
#import <CoreLocation/_CLRangingPeerDistanceInternal.h>
#import <CoreLocation/_CLRangingPeer.h>
#import <CoreLocation/_CLRangingPeerDistance.h>

void* CLClientCreateIso6709Notation(void);
void* CLClientCreateWithBundleIdentifierAndPath(void);
void* CLClientCreateWithBundleIdentifierAndPathOnRunLoop(void);
void* CLClientDisplayStats(void);
void* CLClientGetAuthorizationStatus(void);
void* CLClientGetDistanceCoordinates(void);
void* CLClientGetDistanceHighPrecision(void);
void* CLClientGetError(void);
void* CLClientGetHeading(void);
void* CLClientGetLocation(void);
void* CLClientGetTypeID(void);
void* CLClientInvalidate(void);
void* CLClientIsHeadingAvailable(void);
void* CLClientIsLocationServicesAvailable(void);
void* CLClientIsLocationServicesEnabled(void);
void* CLClientIsValid(void);
void* CLClientLog(void);
void* CLClientLogDump(void);
void* CLClientRequestAuthorization(void);
void* CLClientRetrieveAuthorizationStatus(void);
void* CLClientRetrieveLocation(void);
void* CLClientRetrieveLocationWithDynamicAccuracyReductionAndAlteredAccessoryLocations(void);
void* CLClientSetAllowsMapCorrection(void);
void* CLClientSetCallback(void);
void* CLClientSetShowLocationPrompt(void);
void* CLClientShowHeadingCalibration(void);
void* CLClientShutdownDaemon(void);
void* CLClientStartHeadingUpdates(void);
void* CLClientStartLocationUpdates(void);
void* CLClientStartLocationUpdatesWithDynamicAccuracyReductionAndAlteredAccessoryLocations(void);
void* CLClientStartVehicleHeadingUpdates(void);
void* CLClientStartVehicleSpeedUpdates(void);
void* CLClientStopHeadingUpdates(void);
void* CLClientStopLocationUpdates(void);
void* CLClientStopVehicleHeadingUpdates(void);
void* CLClientStopVehicleSpeedUpdates(void);
void* CLCopyAppsUsingLocation(void);
void* CLCopyTechnologiesInUse(void);
void* CLGenerateRoadSegmentId(void);
void* CLGetControlPlaneStatusReport(void);
void* CLGetLocationDefault(void);
void* CLGetStatusBarIconState(void);
void* CLInternalPerformMigration(void);
void* CLLocationCoordinate2DGetDistanceFrom(void);
void* CLLocationCoordinate2DIsValid(void);
void* CLLocationCoordinate2DMake(void);
void* CLPingDaemon(void);
void* CLSetLocationDefault(void);
void* CLSetMapMatchingRouteHint(void);
void* CLStartStopAdvertisingBeacon(void);

#endif
