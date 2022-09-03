project(SpotlightIndex)

remove_sdk_framework(SpotlightIndex
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1191.2.6")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SpotlightIndex
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SpotlightIndex"
	PRIVATE
)

add_framework(SpotlightIndex
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SpotlightIndex.m

    DEPENDENCIES
        system
        objc
        Foundation
)
