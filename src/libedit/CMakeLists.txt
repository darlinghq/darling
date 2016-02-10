project(libedit)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(-D__APPLE__ -D__MACH__)
add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/local)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ncurses/include)

set(edit_SRCS
	src/chared.c
	src/chartype.c
	src/common.c
	src/el.c
	src/eln.c
	src/emacs.c
	src/filecomplete.c
	src/hist.c
	src/history.c
	src/keymacro.c
	src/map.c
	src/parse.c
	src/prompt.c
	src/read.c
	src/readline.c
	src/refresh.c
	src/search.c
	src/sig.c
	src/terminal.c
	src/tokenizer.c
	src/tty.c
	src/unvis.c
	src/vi.c
	src/vis.c
	src/wcsdup.c

	local/fcns.c
	local/help.c
	local/historyn.c
	local/tokenizern.c
)

add_library(edit SHARED ${edit_SRCS})
target_link_libraries(edit PRIVATE system ncurses)

install(TARGETS edit DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)
