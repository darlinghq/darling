project(AudioUnit)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

option(ENABLE_ALSA "Enable ALSA audio" ON)
option(ENABLE_PULSEAUDIO "Enable PulseAudio" ON)

if (ENABLE_ALSA)
    find_path(ALSA_INCLUDE_DIR asoundlib.h PATH_SUFFIXES alsa)
    message(STATUS "ALSA include dir: ${ALSA_INCLUDE_DIR}")

    if (NOT ALSA_INCLUDE_DIR)
            message(FATAL_ERROR "Cannot find ALSA header files")
    endif (NOT ALSA_INCLUDE_DIR)
endif (ENABLE_ALSA)

if (ENABLE_PULSEAUDIO)
    find_path(PA_INCLUDE_DIR pulse/pulseaudio.h)
    message(STATUS "PA include dir: ${PA_INCLUDE_DIR}")

    if (NOT PA_INCLUDE_DIR)
            message(FATAL_ERROR "Cannot find PulseAudio header files")
    endif (NOT PA_INCLUDE_DIR)
endif (ENABLE_PULSEAUDIO)

#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fblocks -ggdb -O0")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

add_definitions(-DOBJC2RUNTIME -U__APPLE__ -DDISPATCH_NO_INCLUDE_MACH_HEADERS=1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libdispatch)
#include_directories(${DARLING_TOP_DIRECTORY}/src/external/libcxx/include)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/corefoundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libobjc2)
include_directories(${CMAKE_BINARY_DIR}/src/external/corefoundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/foundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc)
include_directories(${ALSA_INCLUDE_DIR} ${PA_INCLUDE_DIR})
include_directories(${DARLING_TOP_DIRECTORY}/basic-headers)
#include_directories(${DARLING_TOP_DIRECTORY}/platform-include)

set(AudioUnit_SRCS
	AUComponent.cpp
	AUGraph.cpp
	AudioUnit.cpp
	AudioOutputUnitComponent.cpp
	AudioUnitBase.cpp
	AudioUnitRenderer.cpp

	AudioQueue.cpp
	AudioQueueBase.cpp
	AudioQueueOutput.cpp
	../util/debug.cpp
	../util/stlutils.cpp
)

if (ENABLE_ALSA)
    add_definitions(-DENABLE_ALSA)
    list(APPEND AudioUnit_SRCS AudioUnitALSA.cpp)
    list(APPEND SoundLibs "-lasound")
endif (ENABLE_ALSA)

if (ENABLE_PULSEAUDIO)
    add_definitions(-DENABLE_PULSEAUDIO)
    list(APPEND AudioUnit_SRCS AudioUnitPA.cpp)
    list(APPEND SoundLibs "-lpulse")
endif (ENABLE_PULSEAUDIO)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(AudioUnit SHARED ${AudioUnit_SRCS})
target_link_libraries(AudioUnit -lstdc++ -lpthread ${SoundLibs} libdispatch_shared CFF)

install(TARGETS AudioUnit DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")

