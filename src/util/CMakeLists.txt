project(util)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(util-SRCS
	log.cpp
	trace.cpp
	stlutils.cpp
	IniConfig.cpp
	leb.cpp
)

add_library(util ${util-SRCS})
set_target_properties(util PROPERTIES COMPILE_FLAGS "-fPIC")

