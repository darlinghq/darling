project(crashhandler)

cmake_minimum_required(VERSION 3.13)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(-fconstant-string-class=NSConstantString)

set(crashhandler_src
	crashhandler.m
)

set_source_files_properties(${crashhandler_src} PROPERTIES LANGUAGE C) # Stupid CMake thinks that ObjC is C++

add_library(crashhandler SHARED ${crashhandler_src})
target_link_libraries(crashhandler -lgnustep-gui -lgnustep-base)

install(TARGETS crashhandler DESTINATION "${CMAKE_INSTALL_LIBDIR}/darling")

