project(IdleTimerServices)

remove_sdk_framework(IdleTimerServices
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(IdleTimerServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/IdleTimerServices"
    PRIVATE
)

add_framework(IdleTimerServices
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/IdleTimerServices.c

    DEPENDENCIES
        system
)
