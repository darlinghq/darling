project(thin)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(AppKit SHARED)
target_link_libraries(AppKit Cocoa)
SET_TARGET_PROPERTIES(AppKit PROPERTIES LINKER_LANGUAGE C)

add_library(auto SHARED)
SET_TARGET_PROPERTIES(auto PROPERTIES LINKER_LANGUAGE C)

install(TARGETS AppKit auto DESTINATION "lib${SUFFIX}/darling")

