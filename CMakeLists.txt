project(darling)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

enable_language(ASM-ATT)
enable_language(ASM_NASM)
ADD_DEFINITIONS(-ggdb -DDEBUG)

if (SUFFIX STREQUAL "32")
	set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -m32")
endif(SUFFIX STREQUAL "32")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/xnu)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/xnu/bsd)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libmach-o)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/util)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

configure_file(config.h.in config.h)

set(util-SRCS
	src/util/log.cc
	src/util/trace.cpp
	src/util/stlutils.cpp
	src/util/IniConfig.cpp
)

add_library(util ${util-SRCS})
set_target_properties(util PROPERTIES COMPILE_FLAGS "-fPIC")

set(mach-o_SRCS
	src/libmach-o/MachO.cpp
	src/libmach-o/FatMachO.cpp
	src/libmach-o/RebaseState.cpp
	src/libmach-o/BindState.cpp
	src/libmach-o/leb.cpp
	src/libmach-o/MachOImpl.cpp
)

add_library(mach-o SHARED ${mach-o_SRCS})
set_target_properties(mach-o PROPERTIES VERSION 1.0.0 SOVERSION 1.0)
set_target_properties(mach-o PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")
target_link_libraries(mach-o -ldl -lpthread util)

set(dyld_SRCS
	src/dyld/FileMap.cpp
	src/dyld/MachOLoader.cpp
	src/dyld/UndefinedFunction.cpp
	src/dyld/Trampoline.cpp
	src/dyld/trampoline_helper.nasm
	src/dyld/ld.cpp
	src/dyld/dyld.cpp
	src/dyld/public.cpp
	src/dyld/keymgr.cpp
	src/dyld/binfmt_misc.cpp
)

add_executable(dyld${SUFFIX} ${dyld_SRCS})
target_link_libraries(dyld${SUFFIX} -ldl -lpthread mach-o util)

set(fatmacho-extract_SRCS
	src/dyld/extract.cpp
)

add_executable(fatmacho-extract ${fatmacho-extract_SRCS})
target_link_libraries(fatmacho-extract -ldl -lpthread -l:./libmach-o.so -Wl,-export-dynamic)

set(motool_SRCS
	src/motool/motool.cpp
)

add_executable(motool ${motool_SRCS})
target_link_libraries(motool mach-o)

install(TARGETS dyld${SUFFIX} DESTINATION bin)

# If building on a 64bit system, only install these for 64bits
if (NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")
	install(TARGETS fatmacho-extract motool DESTINATION bin)
	install(FILES etc/dylib.conf DESTINATION /etc/darling)
endif(NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")

install(TARGETS mach-o DESTINATION lib${SUFFIX})

add_subdirectory(src/libSystem)
add_subdirectory(src/libcxxdarwin)
#add_subdirectory(src/libunwind-darwin-gcc)
add_subdirectory(src/libncurses)
#add_subdirectory(src/CoreServices)

add_dependencies(dyld${SUFFIX} mach-o)

