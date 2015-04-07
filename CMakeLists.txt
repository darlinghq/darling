project(darling-libsystem)

cmake_minimum_required(VERSION 2.4.0)

add_definitions(-D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE -DDARLING)

add_subdirectory(bootstrap_cmds)
add_subdirectory(kernel)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libc/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libm/Source")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/platform-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/platform-include/pthread")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/kernel-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/compiler-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/duct/include")

add_subdirectory(libc)
add_subdirectory(libm)
add_subdirectory(libinfo)
add_subdirectory(libmalloc)
add_subdirectory(libsystem)
add_subdirectory(keymgr)

