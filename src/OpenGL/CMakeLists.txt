project(OpenGL)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


add_library(OpenGL SHARED OpenGL.c)

target_link_libraries(OpenGL GL GLU)
install(TARGETS OpenGL DESTINATION "lib${SUFFIX}/darling")

