project(CoreVideo)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


add_library(CoreVideo SHARED CoreVideo.c)
#target_link_libraries(CoreAudio )

install(TARGETS CoreVideo DESTINATION "lib${SUFFIX}/darling")

