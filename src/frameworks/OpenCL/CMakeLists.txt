project(OpenCL)

remove_sdk_framework(OpenCL)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

include_directories(${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls)

generate_sdk_framework(OpenCL
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/OpenCL"
)

# 32-bit processes keep the framework: they ask for 32-bit host libraries,
# and see CL_PLATFORM_NOT_FOUND_KHR when the host has none.
add_framework(OpenCL
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/OpenCL.m
        src/OpenCLHost.c

    DEPENDENCIES
        system
)

# Contract tests for the shim. opencl_host_shim_test links the real shim with a
# stub elfcalls table, so it checks the typed API and the error paths taken when
# no host ICD loader can be opened, without needing an installed ICD.
# opencl_header_types_test checks the SDK signatures at compile time. Both are
# Mach-O executables: run them in a Darling prefix.
add_darling_executable(opencl_host_shim_test
    ${CMAKE_SOURCE_DIR}/tests/opencl/host_shim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenCLHost.c
)
target_include_directories(opencl_host_shim_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls
)

add_darling_executable(opencl_header_types_test
    ${CMAKE_SOURCE_DIR}/tests/opencl/header_types.cpp
)
target_include_directories(opencl_header_types_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
