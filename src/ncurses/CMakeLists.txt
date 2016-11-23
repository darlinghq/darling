project(ncurses-top)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ncurses)

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
add_definitions(-D _XOPEN_SOURCE_EXTENDED=1)
add_definitions(-DTRACE)

# Hide warnings
add_definitions(
	-Wno-format-security
)

add_subdirectory(ncurses)
add_subdirectory(menu)
add_subdirectory(panel)

install(DIRECTORY terminfo DESTINATION share/darling)
