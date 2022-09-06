project(IOBluetoothUI)

remove_sdk_framework(IOBluetoothUI)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(IOBluetoothUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/IOBluetoothUI"
)

add_framework(IOBluetoothUI
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/IOBluetoothUI.m
        src/AudioOptionsViewInEarDetection.m
        src/AudioOptionsViewMicSwitching.m
        src/AudioOptionsViewListeningMode.m
        src/IOBluetooth_HIDNameFormatter.m
        src/IOBluetoothUI_BatteryControl.m
        src/IOBluetoothUICollectionView.m
        src/IOBluetoothUIDisableProgressBar.m
        src/AudioOptionsViewController.m
        src/IOBluetoothPairingController.m
        src/AudioOptionsViewDoubleTapExtended.m
        src/IOBluetoothServiceBrowserController.m
        src/IOBluetoothDeviceSelectorController.m
        src/IOBluetoothConcreteDeviceSelectorController.m
        src/IOBluetoothConcretePairingController.m
        src/AudioOptionsViewDoubleTap.m
        src/AudioOptions.m
        src/IOBluetoothConcreteServiceBrowserController.m
        src/OBEXObjectReceiveWindowController.m
        src/OBEXObjectReceiveConcreteWindowController.m
        src/TerminateEnabledModalPanel.m
        src/IOBluetoothConcreteDeviceController.m
        src/IOBluetoothTransferProgressUI.m
        src/IOBluetoothConcreteObjectPush.m
        src/IOBluetoothObjectPushUIController.m
        src/IOBluetoothHardwareValidation.m
        src/IOBluetoothDeviceInquiryTableView.m
        src/ObjectTableDataSource.m
        src/AssetHelper.m
        src/PlainFormatter.m
        src/IOBluetoothPasskeyDisplayNibOwner.m
        src/IOBluetoothPasskeyDisplay.m
        src/IOBluetoothAccessibilityIgnoredTextFieldCell.m
        src/IOBluetoothAccessibilityIgnoredImageCell.m
        src/IOBluetoothDeviceImageVault.m
        src/FauxUIElement.m
        src/IOBluetoothUIPairedColorTransformer.m
        src/IOBluetoothUIBool2BWColorTransformer.m
        src/IOBluetoothUIPaired2ActionTitle.m
        src/IOBluetoothUIBackgroundStyle2GrayColorTransformer.m
        src/IOBluetoothUIBackgroundStyle2RemoveButton.m
        src/IOBluetoothUIBackgroundStyle2AlternateRemoveButton.m
        src/IOBluetoothUIConnected2ImageIndicatorTransformer.m
        src/IOBluetoothUIConnected2StatusTransformer.m
        src/AudioOptionsViewClick.m
        src/IOBluetoothDevicePairViewControllerExpansion.m
        src/IOBluetoothDevicePairViewController.m
        src/IOBluetoothUISortedArrayController.m

    DEPENDENCIES
        system
        objc
        Foundation
)
