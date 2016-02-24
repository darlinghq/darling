project(AudioToolbox)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fblocks -ggdb -O0")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

add_definitions(-DOBJC2RUNTIME -U__APPLE__)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/corefoundation/Headers)
include_directories(${CMAKE_BINARY_DIR}/src/external/corefoundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/foundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libobjc2)
include_directories(${DARLING_TOP_DIRECTORY}/src/basic-headers)

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
target_link_libraries(AudioToolbox util -lstdc++ -lavcodec -lavresample -lavformat CFF)

install(TARGETS AudioToolbox DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")

