project(ncurses-top)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ncurses)
include_directories("${CMAKE_SOURCE_DIR}/src/libc/darwin")
#include_directories("${CMAKE_SOURCE_DIR}/src/libc/locale")
#include_directories("${CMAKE_SOURCE_DIR}/src/libc/locale/FreeBSD")
include_directories("${CMAKE_SOURCE_DIR}/src/libc/stdtime/FreeBSD")

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
add_definitions(-D _XOPEN_SOURCE_EXTENDED=1)
#add_definitions(-DTRACE)

# Hide warnings
add_definitions(
	-Wno-format-security
)

set(DYLIB_COMPAT_VERSION "5.4.0")
set(DYLIB_CURRENT_VERSION "5.4.0")

add_subdirectory(ncurses)
add_subdirectory(menu)
add_subdirectory(panel)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/progs)
add_darling_executable(clear progs/clear.c)
target_link_libraries(clear ncurses)

add_library(nc_dump_entry OBJECT progs/dump_entry.c)

add_darling_executable(infocmp progs/infocmp.c $<TARGET_OBJECTS:nc_dump_entry>)
target_link_libraries(infocmp ncurses)

add_darling_executable(tic progs/tic.c $<TARGET_OBJECTS:nc_dump_entry>)
target_link_libraries(tic ncurses)

add_darling_executable(toe progs/toe.c)
target_link_libraries(toe ncurses)

add_darling_executable(tput progs/tput.c)
target_link_libraries(tput ncurses)

add_darling_executable(tset progs/tset.c)
target_link_libraries(tset ncurses)

install(TARGETS clear infocmp tic toe tput tset DESTINATION libexec/darling/usr/bin)

install(DIRECTORY terminfo DESTINATION libexec/darling/usr/share)
