project(CoreBluetooth)

remove_sdk_framework(CoreBluetooth)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreBluetooth
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreBluetooth"
)

add_framework(CoreBluetooth
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/CoreBluetooth.m
        src/BTDevicePicker.m
        src/CBClassicPeer.m
        src/CBCentralManager.m
        src/CBRFCOMMChannel.m
        src/CBCharacteristic.m
        src/CBMutableCharacteristic.m
        src/CBScalablePipeManager.m
        src/CBAttribute.m
        src/CBScalablePipe.m
        src/CBDescriptor.m
        src/CBMutableDescriptor.m
        src/CBPeripheral.m
        src/CBService.m
        src/CBMutableService.m
        src/CBUUID.m
        src/CBL2CAPChannel.m
        src/CBPeer.m
        src/CBClassicManager.m
        src/CBXpcConnection.m
        src/CBPeripheralManager.m
        src/CBCentral.m
        src/CBManager.m
        src/CBATTRequest.m
        src/CBPairingAgent.m

    DEPENDENCIES
        system
        objc
        Foundation
)
