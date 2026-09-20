project(IASUtilities)

remove_sdk_framework(IASUtilities
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "119.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(IASUtilities
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/IASUtilities"
    PRIVATE
)

add_framework(IASUtilities
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/IASUtilities.c

    DEPENDENCIES
        system
)
