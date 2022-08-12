project(ImageCaptureCore)

remove_sdk_framework(ImageCaptureCore)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ImageCaptureCore
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ImageCaptureCore"
)

add_framework(ImageCaptureCore
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ImageCaptureCore.m
        src/ICScannerFeatureProps.m
        src/ICScannerFeatureEnumeration.m
        src/ICScannerFeatureBooleanProps.m
        src/ICScannerBandData.m
        src/ICDeviceBrowser.m
        src/ICScannerFunctionalUnitTransparencyProps.m
        src/ICScannerFunctionalUnitFlatbedProps.m
        src/ICCallbackInfo.m
        src/ICCommand.m
        src/ICCommandCenter.m
        src/ICScannerFeatureEnumerationProps.m
        src/ICScannerFeatureTemplateProps.m
        src/ICDeviceBrowserPrivateData.m
        src/ICScannerImageRep.m
        src/ICClientManager.m
        src/ICClient.m
        src/ICScannerFunctionalUnitDocumentFeederProps.m
        src/ICScannerFeatureRange.m
        src/ICDevice.m
        src/ICCameraDevice.m
        src/ICCameraFile.m
        src/ICAccessManager.m
        src/ICCameraFolder.m
        src/ICCameraItem.m
        src/ICScannerFeatureBoolean.m
        src/ICScannerFunctionalUnit.m
        src/ICScannerFeatureRangeProps.m
        src/ICScannerFunctionalUnitFlatbed.m
        src/ICScannerFunctionalUnitPositiveTransparency.m
        src/ICScannerFunctionalUnitNegativeTransparency.m
        src/ICScannerFeature.m
        src/ICScannerFeatureTemplate.m
        src/ICScannerFunctionalUnitDocumentFeeder.m
        src/ICDeviceAccessManager.m
        src/ICMasterDeviceBrowser.m
        src/ICScannerFunctionalUnitProps.m
        src/ICScannerProperties.m
        src/ICScannerDevice.m

    DEPENDENCIES
        system
        objc
        Foundation
)
