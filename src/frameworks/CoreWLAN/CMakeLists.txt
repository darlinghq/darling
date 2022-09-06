project(CoreWLAN)

remove_sdk_framework(CoreWLAN)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreWLAN
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreWLAN"
)

add_framework(CoreWLAN
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CoreWLAN.m
        src/CWANQPVenueNameEntry.m
        src/CWANQPWANMetrics.m
        src/CWChannel.m
        src/CWConfiguration.m
        src/CWMutableConfiguration.m
        src/CWANQPCapabilityList.m
        src/CWMessageTracerCache.m
        src/CWANQPRoamingConsortium.m
        src/CWWirelessProfile.m
        src/CWANQPConnectionCapability.m
        src/CWANQPNetworkAuthenticationType.m
        src/CWANQPNAIRealm.m
        src/CWANQPNAIRealmEntry.m
        src/CWANQPDomainName.m
        src/CWChannelHistoryItem.m
        src/CWANQP3GPPCellular.m
        src/CWANQPOperatorFriendlyNameEntry.m
        src/CWBaseStationPPPController.m
        src/CWANQPElement.m
        src/WiFiNetworkInternal.m
        src/CWLocationChannel.m
        src/CWTetherDevice.m
        src/CWBTCStatus.m
        src/CWLocationClient.m
        src/CWNetwork.m
        src/CWAutoJoinStatistics.m
        src/CWWiFiClient.m
        src/CWInterfaceManager.m
        src/CWANQPVenueName.m
        src/CWInterface.m
        src/CWANQPOperatorFriendlyName.m
        src/CWNetworkProfile.m
        src/CWMutableNetworkProfile.m
        src/CWANQPProtoPortTuple.m
        src/CWANQPHS20CapabilityList.m
        src/CWEAPOLClient.m
        src/CWPasswordSharingSimulator.m
        src/WiFiDeviceClientInternal.m
        src/CWUnlockSimulator.m
        src/CWRoamStatistics.m
        src/CW8021XProfile.m
        src/CWANQPNetworkAuthenticationTypeEntry.m
        src/CWIPMonitor.m
        src/CWJoinStatistics.m
        src/WiFiManagerClientInternal.m

    DEPENDENCIES
        system
        objc
        Foundation
)
