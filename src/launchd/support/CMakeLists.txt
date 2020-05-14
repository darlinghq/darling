project(launchd-support)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(
	-w -nostdinc
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../liblaunch/)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fblocks -include ${CMAKE_SOURCE_DIR}/src/external/lkm/bsd/sys/fileport.h")

add_definitions(-DPRIVATE)

include(darling_exe)
add_darling_executable(launchctl launchctl.c)
target_link_libraries(launchctl xpc CoreFoundation edit)

add_darling_executable(launchproxy launchproxy.c)
target_link_libraries(launchproxy xpc)

install(TARGETS launchctl DESTINATION libexec/darling/bin)
install(TARGETS launchproxy DESTINATION libexec/darling/usr/libexec)

