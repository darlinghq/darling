project(AGL)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


add_library(AGL SHARED AGL.c)
target_link_libraries(AGL -lGL)

install(TARGETS AGL DESTINATION "lib${SUFFIX}/darling")

