project(FaceTimeTouchBarProtocols)

remove_sdk_framework(FaceTimeTouchBarProtocols
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(FaceTimeTouchBarProtocols
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/FaceTimeTouchBarProtocols"
    PRIVATE
)

add_framework(FaceTimeTouchBarProtocols
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/FaceTimeTouchBarProtocols.c

    DEPENDENCIES
        system
)
