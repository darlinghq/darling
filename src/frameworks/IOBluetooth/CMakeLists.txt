project(IOBluetooth)

remove_sdk_framework(IOBluetooth)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(IOBluetooth
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/IOBluetooth"
)

add_framework(IOBluetooth
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/IOBluetooth.m
        src/IOBluetoothDeviceExpansion.m
        src/SDPQueryCallbackDispatcher.m
        src/IOBluetoothDevice.m
        src/BTClient.m
        src/BluetoothDeviceManager.m
        src/IOBluetoothL2CAPChannelExpansion.m
        src/IOBluetoothL2CAPChannel.m
        src/IOBluetoothRFCOMMChannel.m
        src/IOBluetoothRFCOMMConnection.m
        src/IOBluetoothSDPServiceRecord.m
        src/IOBluetoothSerialPort.m
        src/IOBluetoothSerialPortManager.m
        src/NotificationInfo.m
        src/IOBluetoothNotification.m
        src/IOBluetoothOBEXSession.m
        src/IOBluetoothSDPDataElement.m
        src/IOBluetoothSDPServiceAttribute.m
        src/IOBluetoothSDPUUID.m
        src/OBEXSession.m
        src/IOBluetoothUserNotification.m
        src/IOBluetoothConcreteUserNotification.m
        src/IOBluetoothNSUserNotification.m
        src/IOBluetoothNSObjCUserNotification.m
        src/IOBluetoothNSCUserNotification.m
        src/IOBluetoothLocalSDPServiceRecord.m
        src/IOBluetoothObject.m
        src/OBEXFilePut.m
        src/BluetoothFileReference.m
        src/OBEXFileTransferServices.m
        src/OBEXFileAction.m
        src/OBEXFileGet.m
        src/IOBluetoothCloudServerClient.m
        src/IOBluetoothDaemonNSXPCClient.m
        src/IOBluetoothTransferProgress.m
        src/AppleBluetoothHIDDeviceGen2.m
        src/BluetoothHIDDevice.m
        src/BluetoothHIDDeviceController.m
        src/IOBluetoothDeviceInfoGatherer.m
        src/AppleBluetoothHIDDevice.m
        src/HardcopyCableReplacement.m
        src/IOBluetoothHardcopyCableReplacement.m
        src/RegistryObjectNotificationWrapper.m
        src/IORegistryObjectNotifier.m
        src/SystemPowerNotifier.m
        src/FrameworksAnalyticsPackager.m
        src/IOBluetoothAutomaticDeviceSetup.m
        src/IOBluetoothDeviceInquiryExpansion.m
        src/IOBluetoothDeviceInquiry.m
        src/IOBluetoothDeviceInquiryCSupportObject.m
        src/IOBluetoothHandsFreeAudioGatewayExpansion.m
        src/IOBluetoothHandsFreeAudioGateway.m
        src/IOBluetoothHostControllerExpansion.m
        src/IOBluetoothHostController.m
        src/IOBluetoothDevicePairExpansion.m
        src/IOBluetoothDevicePair.m
        src/BroadcomHostController.m
        src/CSRHostController.m
        src/CSRBlueICEHostController.m
        src/IOBluetoothPreferences.m
        src/IOBluetoothHandsFreeDeviceExpansion.m
        src/IOBluetoothHandsFreeDevice.m
        src/IOBluetoothHandsFreeExpansion.m
        src/IOBluetoothHandsFree.m
        src/BNEPControl.m
        src/BluetoothUIServer.m
        src/AtherosHostController.m
        src/ConnectionCompleteCallbackDispatcher.m
        src/IOBluetoothAudioManager.m
        src/IOBluetoothHeadsetAudioGateway.m

    DEPENDENCIES
        system
        objc
        Foundation
)
