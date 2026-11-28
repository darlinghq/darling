project(NSS)

remove_sdk_framework(NSS)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

# NSS and its host libraries are 64-bit ELF libraries.
set(DARLING_LIB_64BIT_ONLY TRUE)

include_directories(${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls)

generate_sdk_framework(NSS
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/nss"
)

add_framework(NSS
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/NSSHost.c

    DEPENDENCIES
        system
        NSPR
)

# Host-side contract tests. They compile the real shim sources with a stub
# elfcalls table, so no Darling prefix or installed NSS/NSPR is required.
find_package(Threads REQUIRED)

add_executable(nss_nspr_host_shim_test
    ${CMAKE_SOURCE_DIR}/tests/host-shim/host_shim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NSSHost.c
    ${CMAKE_SOURCE_DIR}/src/frameworks/NSPR/src/NSPRHost.c
)
target_include_directories(nss_nspr_host_shim_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/frameworks/NSPR/include
    ${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls
)
target_link_libraries(nss_nspr_host_shim_test PRIVATE Threads::Threads)
add_test(NAME nss_nspr_host_shim COMMAND nss_nspr_host_shim_test)

add_executable(nss_nspr_host_shim_unavailable_test
    ${CMAKE_SOURCE_DIR}/tests/host-shim/host_shim_unavailable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NSSHost.c
    ${CMAKE_SOURCE_DIR}/src/frameworks/NSPR/src/NSPRHost.c
)
target_include_directories(nss_nspr_host_shim_unavailable_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/frameworks/NSPR/include
    ${CMAKE_SOURCE_DIR}/src/startup/mldr/elfcalls
)
target_link_libraries(nss_nspr_host_shim_unavailable_test PRIVATE Threads::Threads)
add_test(NAME nss_nspr_host_shim_unavailable COMMAND nss_nspr_host_shim_unavailable_test)
