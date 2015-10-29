project(motool)

cmake_minimum_required(VERSION 2.4.0)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmach-o)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


set(motool_SRCS
	motool.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(motool ${motool_SRCS})
target_link_libraries(motool mach-o)

install(TARGETS motool DESTINATION bin)

