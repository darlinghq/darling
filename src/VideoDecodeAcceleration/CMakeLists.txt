project(VideoDecodeAcceleration)

cmake_minimum_required(VERSION 3.10)

enable_language(C ASM)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_definitions(-DOBJC2RUNTIME=1)

set(vda_SRCS
	dummy.c
)

add_library(VideoDecodeAcceleration SHARED ${vda_SRCS})
target_link_libraries(VideoDecodeAcceleration PRIVATE system CFF)

install(TARGETS VideoDecodeAcceleration DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)
