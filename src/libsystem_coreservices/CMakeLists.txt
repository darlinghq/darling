project(system_coreservices)

cmake_minimum_required(VERSION 2.4.0)

if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 32)
	add_definitions(-DTARGET_CPU_X86=1)
else(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 64)
	add_definitions(-DTARGET_CPU_X86_64=1)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall/wrappers)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_coreservices.dylib")
add_darling_library(system_coreservices SHARED dirhelper.c NSSystemDirectories.c)
target_link_libraries(system_coreservices PRIVATE system_c system_dyld platform system_pthread system_kernel)
make_fat(system_coreservices)

install(TARGETS system_coreservices DESTINATION libexec/darling/usr/lib/system)
