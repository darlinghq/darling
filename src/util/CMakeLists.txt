project(util)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(util-SRCS
	debug.cpp
	stlutils.cpp
	IniConfig.cpp
	leb.cpp
)

#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(darling-util ${util-SRCS})
set_target_properties(darling-util PROPERTIES COMPILE_FLAGS "-fPIC")
#install(TARGETS darling-util DESTINATION lib${SUFFIX}/darling)

