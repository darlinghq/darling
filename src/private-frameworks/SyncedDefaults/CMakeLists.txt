project(SyncedDefaults)

remove_sdk_framework(SyncedDefaults
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SyncedDefaults
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SyncedDefaults"
    PRIVATE
)

add_framework(SyncedDefaults
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SyncedDefaults.c

    DEPENDENCIES
        system
)
