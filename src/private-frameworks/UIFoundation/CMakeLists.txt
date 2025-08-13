project(UIFoundation)

remove_sdk_framework(UIFoundation
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(UIFoundation
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/UIFoundation"
	PRIVATE
)

add_framework(UIFoundation
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/UIFoundation.m
        src/UINibDecoder.m
        src/UINibEncoder.m

    DEPENDENCIES
        system
        objc
        CoreFoundation
        Foundation
)
