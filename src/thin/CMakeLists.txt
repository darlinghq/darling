project(thin)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_library(AppKit SHARED)
target_link_libraries(AppKit -lgnustep-gui)
SET_TARGET_PROPERTIES(AppKit PROPERTIES LINKER_LANGUAGE C)

add_library(auto SHARED)
SET_TARGET_PROPERTIES(auto PROPERTIES LINKER_LANGUAGE C)

install(TARGETS AppKit auto DESTINATION "lib${SUFFIX}/darling")

