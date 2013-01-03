project(motool)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


set(motool_SRCS
	motool.cpp
)

add_executable(motool ${motool_SRCS})
target_link_libraries(motool mach-o)

