project(AudioUnit)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fblocks")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)

set(AudioUnit_SRCS
	AUComponent.cpp
	AUGraph.cpp
	AudioUnit.cpp
	AudioOutputUnitComponent.cpp
	AudioUnitBase.cpp
	AudioUnitALSA.cpp
	AudioUnitRenderer.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(AudioUnit SHARED ${AudioUnit_SRCS})
target_link_libraries(AudioUnit util -lasound -ldispatch)

install(TARGETS AudioUnit DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")

