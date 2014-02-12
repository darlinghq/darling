project(libdyld)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../util)
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
)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
	set(dyld_SRCS ${dyld_SRCS}
		UndefinedFunction.cpp
		Trampoline.cpp
		trampoline_helper.nasm
		dyld_stub_binder.nasm
		tls_helper.nasm
		TLS.cpp
	)

	add_definitions(-DHAS_DEBUG_HELPERS)
	add_definitions(-DHAS_TLS_SUPPORT)

endif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(dyld SHARED ${dyld_SRCS})
target_link_libraries(dyld -ldl -lpthread mach-o util)

install(TARGETS dyld DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)


