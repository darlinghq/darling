project(dyld-bin)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(dyld_SRCS
	dyld.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(dyld-bin ${dyld_SRCS})
set_target_properties(dyld-bin PROPERTIES OUTPUT_NAME dyld${SUFFIX})
target_link_libraries(dyld-bin -l:../libdyld/libdyld.so util mach-o)

if (NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")
	add_executable(dyldd dyldd.cpp)
	target_link_libraries(dyldd -l:../libdyld/libdyld.so util mach-o)
	install(TARGETS dyldd DESTINATION bin)
endif (NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")

install(TARGETS dyld-bin DESTINATION bin)

if (SUFFIX STREQUAL "64")
	add_executable(dyld-multilib dyld-multilib.c)
	set_target_properties(dyld-multilib PROPERTIES OUTPUT_NAME dyld)
	install(TARGETS dyld-multilib DESTINATION bin)
endif (SUFFIX STREQUAL "64")

