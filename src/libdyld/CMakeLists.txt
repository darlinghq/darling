project(libdyld)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(dyld_SRCS
	eh/BufReWriter.cpp
	eh/CFIWalker.cpp
	eh/DwarfPointer.cpp
	eh/BufWriter.cpp
	eh/BufReader.cpp
	eh/EHSection.cpp
	Trampoline.cpp
	trampoline_helper.nasm
	dyld_stub_binder.nasm
	tls_helper.nasm
	dyld_public.cpp
	dl_public.cpp
	binfmt_misc.cpp
	TLS.cpp
	
	MachOObject.cpp
	LoadableObject.cpp
	NativeObject.cpp
	MachOMgr.cpp
	DylibSearch.cpp
	UndefinedFunction.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(dyld SHARED ${dyld_SRCS})
target_link_libraries(dyld -ldl -lpthread mach-o util)

install(TARGETS dyld DESTINATION lib${SUFFIX}/darling)


