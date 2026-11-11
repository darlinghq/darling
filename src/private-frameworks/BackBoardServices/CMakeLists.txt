project(BackBoardServices)

remove_sdk_framework(BackBoardServices
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(BackBoardServices
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/BackBoardServices"
    PRIVATE
)

add_framework(BackBoardServices
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/BackBoardServices.c

    DEPENDENCIES
        system
)
