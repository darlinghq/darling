project(CoreBluetooth)

remove_sdk_framework(CoreBluetooth)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The BlueZ transport talks to the host system bus through Darling's shared
# libdbus wrapper, so it needs both the wrapper's header and dbus-1's headers.
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_DBUS REQUIRED dbus-1)
include_directories(${CMAKE_SOURCE_DIR}/src/libdarlingdbus/include)
include_directories(${PC_DBUS_INCLUDE_DIRS})

# The BlueZ bridge is internal to the framework: its C++ namespace and the
# bridge class must not become exported symbols of CoreBluetooth.
set_source_files_properties(
    src/bluez_wire.cpp
    src/CBBlueZTransport.mm
    src/CBHostSession.mm
    PROPERTIES COMPILE_OPTIONS "-fvisibility=hidden"
)

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
        src/bluez_wire.cpp
        src/CBBlueZTransport.mm
        src/CBHostSession.mm
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
        src/CBError.m

    DEPENDENCIES
        system
        objc
        Foundation
        cxx
        darling_dbus_loader
)
