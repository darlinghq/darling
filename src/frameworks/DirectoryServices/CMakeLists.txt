project(DirectoryService)

remove_sdk_framework(DirectoryService)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DirectoryService
    VERSION ${FRAMEWORK_VERSION}
    HEADER "darling-framework/include/DirectoryService"
)

add_framework(DirectoryService
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        ${CMAKE_SOURCE_DIR}/src/external/OpenDirectory/DirectoryService/DirServices.c
        ${CMAKE_SOURCE_DIR}/src/external/OpenDirectory/DirectoryService/DirServicesCustom.c
        ${CMAKE_SOURCE_DIR}/src/external/OpenDirectory/DirectoryService/DirServicesUtils.c

    DEPENDENCIES
        system
        CoreFoundation
        CFOpenDirectory
)
