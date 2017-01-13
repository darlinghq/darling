project(dyld-apple)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include (darling_exe)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc -std=c++11 -include ${CMAKE_SOURCE_DIR}/src/duct/include/CrashReporterClient.h")
set(CMAKE_SHARED_LINKER_FLAGS_SAVED "${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,-flat_namespace -Wl,-undefined,suppress")

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)

include_directories(${CMAKE_SOURCE_DIR}/src/launchd/liblaunch)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/gen)
include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/sandbox)
include_directories(${CMAKE_SOURCE_DIR}/src/external/libcxx/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/launch-cache)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include/mach-o)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libdyld.dylib")
add_darling_library(system_dyld SHARED
	src/dyldAPIsInLibSystem.cpp
	src/dyld_stub_binder.S
	src/dyldLibSystemGlue.c
	src/threadLocalHelpers.S
	src/dyldLock.cpp
	src/threadLocalVariables.c
	src/start_glue.S
)
set_target_properties(system_dyld PROPERTIES
	OUTPUT_NAME "dyld"
	COMPILE_FLAGS "-fno-exceptions -fno-rtti")
target_link_libraries(system_dyld PRIVATE system_c system_kernel compiler_rt launch)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS_SAVED} -nostdlib")

add_darling_executable(system_loader
	src/dyldAPIs.cpp
	src/dyld.cpp
	src/dyld_gdb.cpp
	src/dyldInitialization.cpp
	src/dyldNew.cpp
	#src/dyld_process_info.cpp
	#src/dyld_process_info_notify.cpp
	src/ImageLoader.cpp
	src/ImageLoaderMachOClassic.cpp
	src/ImageLoaderMachOCompressed.cpp
	src/ImageLoaderMachO.cpp
	src/ImageLoaderMegaDylib.cpp
	src/dyldExceptions.c
	src/glue.c
	src/dyldStartup.S
	src/stub_binding_helper.S
	src/start_glue.S
)

#use_ld64(system_loader)
set_target_properties(system_loader PROPERTIES OUTPUT_NAME "dyld")
set_property(TARGET system_loader APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-dylinker -Wl,-dead_strip -nostdlib -Wl,-e,__dyld_start")
target_link_libraries(system_loader libc_static system_blocks_static unwind_static system_kernel_static system_m_static cxxabi_static keymgr_static compiler_rt_static system_duct_static)

install(TARGETS system_dyld DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)
