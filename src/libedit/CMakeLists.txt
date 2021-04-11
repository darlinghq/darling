project(libedit)

cmake_minimum_required(VERSION 3.10)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/local)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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

set(DYLIB_INSTALL_NAME "/usr/lib/libedit.3.dylib")
set(DYLIB_COMPAT_VERSION "2.0.0")
set(DYLIB_CURRENT_VERSION "3.0.0")

add_darling_library(edit SHARED ${edit_SRCS})
make_fat(edit)
set_target_properties(edit PROPERTIES OUTPUT_NAME "edit.3")
target_link_libraries(edit PRIVATE system ncurses)

InstallSymlink("libedit.3.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libedit.2.dylib")
InstallSymlink("libedit.3.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libedit.dylib")

install(TARGETS edit DESTINATION libexec/darling/usr/lib)
install(FILES inputrc DESTINATION libexec/darling/etc)
