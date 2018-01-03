project(SpotlightReceiver)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(SpotlightReceiver
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

    SOURCES
        src/SpotlightReceiver.m
        src/CSRecieverConnection.m

    DEPENDENCIES
        system
        objc
        Foundation
)
