project(dyld-bin)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

enable_language(C ASM)

set(dyld_SRCS
	dyld.cpp
	dirstructure.cpp
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U__APPLE__ -std=c++11 -stdlib=libc++ -ggdb -O0")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Needed because libdyld references pthread funcs in libsystem_c.so
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--unresolved-symbols=ignore-in-shared-libs")

add_definitions(-D_GNU_SOURCE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmach-o)

# if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")

    add_definitions(-DHAS_DEBUG_HELPERS)
    add_definitions(-DHAS_TLS_SUPPORT)

# endif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")


add_executable(dyld-bin ${dyld_SRCS})
set_target_properties(dyld-bin PROPERTIES OUTPUT_NAME dyld${SUFFIX})
target_link_libraries(dyld-bin dyld darling-util mach-o)

if (NOT DARLING_NO_EXECUTABLES)
	add_executable(dyldd dyldd.cpp)
	target_link_libraries(dyldd dyld darling-util mach-o)
	install(TARGETS dyldd DESTINATION bin)
    install(PROGRAMS darling DESTINATION bin
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
                GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
	add_library(dexe_startup STATIC darling-so-start.S)
endif (NOT DARLING_NO_EXECUTABLES)

	install(TARGETS dyld-bin DESTINATION bin)

if (BITS EQUAL 64)
	add_executable(dyld-multilib dyld-multilib.c)
	set_target_properties(dyld-multilib PROPERTIES OUTPUT_NAME dyld)
	install(TARGETS dyld-multilib DESTINATION bin)
endif (BITS EQUAL 64)


