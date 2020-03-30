project(system_coreservices)

cmake_minimum_required(VERSION 2.4.0)

add_definitions(-DTARGET_OS_MAC=1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")

include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall/wrappers)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_coreservices.dylib")
add_darling_library(system_coreservices SHARED dirhelper.c NSSystemDirectories.c sysdir.c)
target_link_libraries(system_coreservices PRIVATE system_c system_dyld platform system_pthread system_kernel)
make_fat(system_coreservices)

install(TARGETS system_coreservices DESTINATION libexec/darling/usr/lib/system)
