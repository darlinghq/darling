project(QuickTime)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


add_library(QuickTime SHARED QuickTime.c)


install(TARGETS QuickTime DESTINATION "lib${SUFFIX}/darling")

