project(DistributedEvaluation)

remove_sdk_framework(DistributedEvaluation
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DistributedEvaluation
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DistributedEvaluation"
    PRIVATE
)

add_framework(DistributedEvaluation
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/DistributedEvaluation.c

    DEPENDENCIES
        system
)
