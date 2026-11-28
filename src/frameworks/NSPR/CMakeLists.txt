project(NSPR)

remove_sdk_framework(NSPR)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

# The host libnspr4 is a 64-bit ELF library; a 32-bit slice could not load it.
set(DARLING_LIB_64BIT_ONLY TRUE)

include_directories(${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls)

generate_sdk_framework(NSPR
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/nspr"
)

add_framework(NSPR
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/NSPRHost.c

    DEPENDENCIES
        system
)
