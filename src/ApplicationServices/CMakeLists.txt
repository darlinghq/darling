project(ApplicationServices)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(ApplicationServices SHARED $<TARGET_OBJECTS:coregraphics>)
target_link_libraries(ApplicationServices system CoreServices lcms png12 gif tiff jpeg X11 Xext Xrender fontconfig cairo)

install(TARGETS ApplicationServices DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")

