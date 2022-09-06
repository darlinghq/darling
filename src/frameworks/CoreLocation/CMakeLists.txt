project(CoreLocation)

remove_sdk_framework(CoreLocation)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "2245.16.14")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreLocation
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreLocation"
)

add_framework(CoreLocation
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CoreLocation.m
        src/_CLLocationManagerRoutineProxy.m
        src/CLLocationManagerRoutine.m
        src/CLEmergencyEnablementAssertion.m
        src/CLLocationManagerStateTracker.m
        src/CLLocationManagerInternal.m
        src/CLLocationManager.m
        src/CLHeadingInternal.m
        src/CLHeading.m
        src/CLRegionInternal.m
        src/CLRegion.m
        src/CLCircularRegion.m
        src/CLBeaconRegion.m
        src/CLBeaconInternal.m
        src/CLBeacon.m
        src/CLSimulationManager.m
        src/CLGeocoderInternal.m
        src/CLGeocoder.m
        src/CLLocationInternalClient.m
        src/CLPlacemarkInternal.m
        src/CLPlacemark.m
        src/CLHarvester.m
        src/CLStateTracker.m
        src/CLLocationInternal.m
        src/CLLocation.m
        src/CLAssertion.m
        src/CLTilesManagerClientInternal.m
        src/CLTilesManagerClient.m
        src/_CLRangingPeerInternal.m
        src/_CLRangingPeerDistanceInternal.m
        src/_CLRangingPeer.m
        src/_CLRangingPeerDistance.m

    DEPENDENCIES
        system
        objc
        Foundation
)
