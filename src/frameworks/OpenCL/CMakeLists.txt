project(OpenCL)

remove_sdk_framework(OpenCL)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

# The host ICD loader is a 64-bit ELF library. As with OpenGLES, there is no
# useful 32-bit slice: a 32-bit process could not load the backend.
set(DARLING_LIB_64BIT_ONLY TRUE)

include_directories(${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls)

generate_sdk_framework(OpenCL
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/OpenCL"
)

add_framework(OpenCL
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/OpenCL.m
        src/OpenCLHost.c

    DEPENDENCIES
        system
)

# Host-side contract test. It links the real shim with a stub elfcalls table,
# so it checks the typed API and the error paths taken when no host ICD loader
# can be opened without needing a Darling prefix or an installed ICD.
find_package(Threads REQUIRED)
add_executable(opencl_host_shim_test
    ${CMAKE_SOURCE_DIR}/tests/opencl/host_shim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenCLHost.c
)
target_include_directories(opencl_host_shim_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls
)
target_link_libraries(opencl_host_shim_test PRIVATE Threads::Threads)
add_test(NAME opencl_host_shim COMMAND opencl_host_shim_test)

add_executable(opencl_header_types_test
    ${CMAKE_SOURCE_DIR}/tests/opencl/header_types.cpp
)
target_include_directories(opencl_header_types_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_test(NAME opencl_header_types COMMAND opencl_header_types_test)
