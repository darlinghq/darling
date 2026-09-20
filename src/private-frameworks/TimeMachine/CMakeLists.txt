project(TimeMachine)

remove_sdk_framework(TimeMachine
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(TimeMachine
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/TimeMachine"
    PRIVATE
)

add_framework(TimeMachine
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/TimeMachine.c

    DEPENDENCIES
        system
)
