project(NewDeviceOutreachMacUI)

remove_sdk_framework(NewDeviceOutreachMacUI
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(NewDeviceOutreachMacUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/NewDeviceOutreachMacUI"
    PRIVATE
)

add_framework(NewDeviceOutreachMacUI
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/NewDeviceOutreachMacUI.c

    DEPENDENCIES
        system
)
