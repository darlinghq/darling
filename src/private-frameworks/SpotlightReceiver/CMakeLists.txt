project(SpotlightReceiver)

remove_sdk_framework(SpotlightReceiver
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SpotlightReceiver
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SpotlightReceiver"
	PRIVATE
)

add_framework(SpotlightReceiver
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SpotlightReceiver.m
        src/CSRecieverConnection.m

    DEPENDENCIES
        system
        objc
        Foundation
)
