project(DeviceCheck)

remove_sdk_framework(DeviceCheck)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DeviceCheck
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DeviceCheck"
)

add_framework(DeviceCheck
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/DeviceCheck.c

    DEPENDENCIES
        system
)
