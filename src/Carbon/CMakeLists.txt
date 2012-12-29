project(Carbon)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#configure_file(config.h.in config.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)

set(Carbon_SRCS
	HIToolbox/MacWindows.cpp
)

add_library(Carbon SHARED ${Carbon_SRCS})
target_link_libraries(Carbon util CoreServices -lX11)

install(TARGETS Carbon DESTINATION "lib${SUFFIX}/darling")

