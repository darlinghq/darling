project(libsandbox)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(-D__APPLE__ -D__MACH__)
add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_sandbox.dylib")
add_darling_library(sandbox SHARED sandbox.c)
target_link_libraries(sandbox PRIVATE system_c system_kernel system_dyld system_malloc)
set_target_properties(sandbox PROPERTIES OUTPUT_NAME "system_sandbox")

install(TARGETS sandbox DESTINATION libexec/darling/usr/lib/system)
