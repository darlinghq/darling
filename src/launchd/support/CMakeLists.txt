project(launchd-support)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(
	-Wno-extra-tokens
	-Wno-pointer-bool-conversion
	-Wno-implicit-function-declaration
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/private)
include_directories(${CMAKE_SOURCE_DIR}/platform-include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../liblaunch/)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/gen)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/include/NetBSD)
include_directories(${CMAKE_SOURCE_DIR}/src/libedit/src)
include_directories(${CMAKE_SOURCE_DIR}/src/libinfo/lookup.subproj)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/external/corefoundation)
include_directories(${CMAKE_SOURCE_DIR}/src/external/foundation/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fblocks -nostdinc -include ${CMAKE_SOURCE_DIR}/kernel-include/sys/fileport.h")

add_definitions(-DPRIVATE)

include(darling_exe)
add_darling_executable(launchctl launchctl.c)
target_link_libraries(launchctl xpc CoreFoundation edit)

install(TARGETS launchctl DESTINATION libexec/darling/bin)

