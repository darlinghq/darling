project(Accelerate)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "4.0.0")

remove_sdk_framework(Accelerate)

add_subdirectory(vecLib)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(Accelerate
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Accelerate"
)

add_framework(Accelerate
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Accelerate.c

    DEPENDENCIES
        system
)

reexport(Accelerate vecLib ${CMAKE_CURRENT_BINARY_DIR}/vecLib/vecLib)
