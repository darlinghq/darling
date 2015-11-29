project(libdyld)

cmake_minimum_required(VERSION 2.4.0)

# set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf") # TODO: for 32bit
enable_language(ASM)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb -O0")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/darwin.map")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmach-o)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/xnu)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/xnu/bsd)

set(dyld_SRCS
	eh/BufReWriter.cpp
	eh/CFIWalker.cpp
	eh/DwarfPointer.cpp
	eh/BufWriter.cpp
	eh/BufReader.cpp
	eh/EHSection.cpp
	dyld_public.cpp
	dl_public.cpp
	binfmt_misc.cpp
	
	MachOObject.cpp
	LoadableObject.cpp
	NativeObject.cpp
	MachOMgr.cpp
	DylibSearch.cpp
	environ.c
	threads.cpp
)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
	set(dyld_SRCS ${dyld_SRCS}
		UndefinedFunction.cpp
		Trampoline.cpp
		x86-64/dyld_stub_binder.S
		x86-64/trampoline_helper.S
		x86-64/tls_helper.S
		i386/dyld_stub_binder.S
		i386/trampoline_helper.S
		i386/tls_helper.S
		TLS.cpp
	)

	add_definitions(-DHAS_DEBUG_HELPERS)
	add_definitions(-DHAS_TLS_SUPPORT)

endif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set_source_files_properties(environ.c COMPILE_FLAGS "-no-integrated-as")

add_library(dyld SHARED ${dyld_SRCS})
target_link_libraries(dyld PRIVATE -ldl -lpthread mach-o darling-util)

install(TARGETS dyld DESTINATION lib${SUFFIX}/darling)


