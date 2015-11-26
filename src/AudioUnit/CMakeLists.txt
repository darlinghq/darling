project(AudioUnit)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_path(ALSA_INCLUDE_DIR asoundlib.h PATH_SUFFIXES alsa)
message(STATUS "ALSA include dir: ${ALSA_INCLUDE_DIR}")

if (NOT ALSA_INCLUDE_DIR)
	message(FATAL_ERROR "Cannot find ALSA header files")
endif (NOT ALSA_INCLUDE_DIR)

#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fblocks -nostdinc++ -ggdb -O0")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map -nodefaultlibs")

add_definitions(-DOBJC2RUNTIME -U__APPLE__)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libdispatch)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libcxx/include)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/corefoundation/Headers)
include_directories(${CMAKE_BINARY_DIR}/src/external/corefoundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/foundation/Headers)
include_directories(${ALSA_INCLUDE_DIR})
#include_directories(${DARLING_TOP_DIRECTORY}/platform-include)

set(AudioUnit_SRCS
	AUComponent.cpp
	AUGraph.cpp
	AudioUnit.cpp
	AudioOutputUnitComponent.cpp
	AudioUnitBase.cpp
	AudioUnitALSA.cpp
	AudioUnitRenderer.cpp
        # AudioUnitPA.cpp

        AudioQueue.cpp
        AudioQueueBase.cpp
        AudioQueueOutput.cpp
	../util/debug.cpp
	../util/stlutils.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(AudioUnit SHARED ${AudioUnit_SRCS})
target_link_libraries(AudioUnit -lc -lasound libdispatch_shared CFF cxx)

install(TARGETS AudioUnit DESTINATION "lib${SUFFIX}/darling")

