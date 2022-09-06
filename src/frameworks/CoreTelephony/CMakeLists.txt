project(CoreTelephony)

remove_sdk_framework(CoreTelephony)

set(FRAMEWORK_VERSION "A")

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "0.0.0")

generate_sdk_framework(CoreTelephony
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreTelephony"
)

add_framework(CoreTelephony
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CoreTelephony.m
        src/CTIMSRegistrationStatus.m
        src/CTAsciiAddress.m
        src/CTRadioAccessTechnology.m
        src/CTBundle.m
        src/CTMessage.m
        src/CTSimLabel.m
        src/CTPhoneNumber.m
        src/CTSupportedMaxDataRates.m
        src/CTRegistrationDisplayStatus.m
        src/CTCallCapabilities.m
        src/CTMmsRegistrationFailureInfoType.m
        src/CTDataStatus.m
        src/CTMessageCenter.m
        src/CTMmsEncoder.m
        src/CTEncryptionStatusInfo.m
        src/CTMessagePart.m
        src/CTIMSRegistrationTransportInfo.m
        src/CTCallCenter.m
        src/CTPhoneBookEntry.m
        src/CTSuppServicesNotificationData.m
        src/CTCallForwardingValue.m
        src/CTMessageStatus.m
        src/CTVoicemailInfoType.m
        src/CTTelephonyNetworkInfo.m
        src/CTEmailAddress.m
        src/CTEmergencyMode.m
        src/CTMobileEquipmentInfo.m
        src/CTMobileEquipmentInfoList.m
        src/CTSubscriberInfo.m
        src/CTEmergencyModeResult.m
        src/CTXPCServiceSubscriptionContext.m
        src/CTXPCServiceSubscriptionInfo.m
        src/CTXPCSimLessContextInfo.m
        src/CTXPCSimLessContexts.m
        src/CTXPCContextInfo.m
        src/CTXPCContexts.m
        src/CTDataSettings.m
        src/CTAudioCodecInfo.m
        src/CTCarrier.m
        src/CTSignalStrengthInfo.m
        src/CTSignalStrengthMeasurements.m
        src/CTCellInfo.m
        src/CTCall.m
        src/CTCellIdInfo.m
        src/CTBinarySMS.m
        src/CTDataConnectionAvailabilityStatus.m
        src/CTDataUsage.m
        src/CTDataUsed.m
        src/CTAppDataUsage.m
        src/CTPerAppDataUsage.m
        src/CTDeviceDataUsage.m
        src/CTServiceDescriptor.m
        src/CTServiceDescriptorContainer.m
        src/CTCellularPlanProvisioningRequest.m
        src/CTVoiceLinkQualityMetric.m
        src/CTEnhancedLinkQualityMetric.m
        src/CTEnhancedDataLinkQualityMetric.m
        src/CoreTelephonyClientRemoteAsyncProxy.m
        src/CTNetworkList.m
        src/CoreTelephonyClientDelegateProxy.m
        src/CTXPCError.m
        src/CTNetwork.m
        src/CTDataConnectionStatus.m
        src/CTPNRDataType.m
        src/CTPNRRequestType.m
        src/CTPNRRequestSentInfo.m
        src/CTPNRContextInfo.m
        src/MuxNotificationSink.m
        src/CoreTelephonyClientMux.m
        src/CTSubscriberAlgorithm.m
        src/CTSubscriberAlgorithmEAPSIM.m
        src/CTSubscriberAlgorithmEAPAKA.m
        src/CTSubscriberAuthRequest.m
        src/CTSubscriberAuthResult.m
        src/CoreTelephonyClient.m
        src/CTNetworkSelectionInfo.m
        src/CTCellularPlanManagerCameraScanAction.m
        src/CTSubscriberAuthDataHolder.m
        src/CTPhoneNumberInfo.m
        src/CTPriVersion.m
        src/CTSMSDataType.m

    DEPENDENCIES
        system
        objc
        Foundation
)
