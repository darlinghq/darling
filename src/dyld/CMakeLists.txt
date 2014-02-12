project(dyld-bin)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(dyld_SRCS
	dyld.cpp
	dirstructure.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")

    add_definitions(-DHAS_DEBUG_HELPERS)
    add_definitions(-DHAS_TLS_SUPPORT)

endif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")


add_executable(dyld-bin ${dyld_SRCS})
set_target_properties(dyld-bin PROPERTIES OUTPUT_NAME dyld${SUFFIX})
target_link_libraries(dyld-bin dyld util mach-o)

#if (NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")
	add_executable(dyldd dyldd.cpp)
	target_link_libraries(dyldd dyld util mach-o)
	install(TARGETS dyldd DESTINATION bin)
	#endif (NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")

install(TARGETS dyld-bin DESTINATION bin)

if (CMAKE_INSTALL_LIBDIR STREQUAL "lib64")
	add_executable(dyld-multilib dyld-multilib.c)
	set_target_properties(dyld-multilib PROPERTIES OUTPUT_NAME dyld)
	install(TARGETS dyld-multilib DESTINATION bin)
endif (CMAKE_INSTALL_LIBDIR STREQUAL "lib64")

