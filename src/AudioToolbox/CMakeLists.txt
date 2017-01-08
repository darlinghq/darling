project(AudioToolbox)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fblocks -ggdb -O0")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

add_definitions(-DOBJC2RUNTIME -U__APPLE__ -DDISPATCH_NO_INCLUDE_MACH_HEADERS=1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/corefoundation/Headers)
include_directories(${CMAKE_BINARY_DIR}/src/external/corefoundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/foundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libobjc2)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc)
include_directories(${DARLING_TOP_DIRECTORY}/basic-headers)

include(CheckSymbolExists)

set(CMAKE_REQUIRED_LIBRARIES -lavutil)
check_symbol_exists(av_frame_alloc "libavutil/frame.h" HAVE_AV_FRAME_ALLOC)
if (HAVE_AV_FRAME_ALLOC)
	add_definitions(-DHAVE_AV_FRAME_ALLOC)
endif(HAVE_AV_FRAME_ALLOC)

set(AudioToolbox_SRCS
	AudioConverter.cpp
	AudioConverterImpl.cpp
	AudioFile.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(AudioToolbox SHARED ${AudioToolbox_SRCS})
target_link_libraries(AudioToolbox darling-util -lstdc++ -lavcodec -lavresample -lavformat CFF)

install(TARGETS AudioToolbox DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")

