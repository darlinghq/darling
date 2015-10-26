project(libncurses)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

enable_language(ASM_NASM)

#if (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")
#	message(FATAL_ERROR "Clang is the only supported compiler.")
#endif (NOT "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang")

#configure_file(config.h.in config.h)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fblocks -std=c++0x")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(libncurses_SRCS
	wrap.c
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(ncursesdarwin SHARED ${libncurses_SRCS})
# -luuid to make uuid_ functions available for Darwin apps
target_link_libraries(ncursesdarwin -lncurses)

install(TARGETS ncursesdarwin DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")

