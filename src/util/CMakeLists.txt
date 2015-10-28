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
	Regexp.cpp
	leb.cpp
)

add_library(darling-util ${util-SRCS})
set_target_properties(darling-util PROPERTIES COMPILE_FLAGS "-fPIC")

