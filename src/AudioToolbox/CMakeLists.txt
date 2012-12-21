project(AudioToolbox)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


add_library(AudioToolbox SHARED AudioToolbox.c)


install(TARGETS AudioToolbox DESTINATION "lib${SUFFIX}/darling")

