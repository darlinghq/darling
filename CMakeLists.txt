project(darling)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if (SUFFIX)
	set(MULTILIB TRUE)
endif (SUFFIX)
if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR SUFFIX STREQUAL "32")
	set(BITS 32)
else(CMAKE_SIZEOF_VOID_P EQUAL 4 OR SUFFIX STREQUAL "32")
	set(BITS 64)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4 OR SUFFIX STREQUAL "32")

message(STATUS "This is a ${BITS}-bit build")

if (SUFFIX STREQUAL "32")
	set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -m32")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf")
	SET(CMAKE_C_SIZEOF_DATA_PTR 4)
endif(SUFFIX STREQUAL "32")

enable_language(ASM-ATT)
enable_language(ASM_NASM)
ADD_DEFINITIONS(-ggdb -DDEBUG)
#ADD_DEFINITIONS(-O3)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/xnu)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/xnu/bsd)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libmach-o)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/util)
include_directories(${CMAKE_BINARY_DIR}) # for config.h
include_directories(/usr/include/GNUstep)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

configure_file(config.h.in config.h)

add_executable(genfuncmap src/util/genfuncmap.cpp)

set(util-SRCS
	src/util/log.cpp
	src/util/trace.cpp
	src/util/stlutils.cpp
	src/util/IniConfig.cpp
	src/util/leb.cpp
)

add_library(util ${util-SRCS})
set_target_properties(util PROPERTIES COMPILE_FLAGS "-fPIC")

set(mach-o_SRCS
	src/libmach-o/MachO.cpp
	src/libmach-o/FatMachO.cpp
	src/libmach-o/RebaseState.cpp
	src/libmach-o/BindState.cpp
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
	src/dyld/dyld_stub_binder.nasm
	src/dyld/ld.cpp
	src/dyld/dyld.cpp
	src/dyld/public.cpp
	src/dyld/binfmt_misc.cpp
	src/dyld/ELFBlock.cpp
	src/dyld/GDBInterface.cpp
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")

add_executable(dyld${SUFFIX} ${dyld_SRCS})
target_link_libraries(dyld${SUFFIX} -ldl -lpthread mach-o util)

#set(fatmacho-extract_SRCS
#	src/dyld/extract.cpp
#)

#add_executable(fatmacho-extract ${fatmacho-extract_SRCS})
#target_link_libraries(fatmacho-extract -ldl -lpthread -l:./libmach-o.so -Wl,-export-dynamic)

set(motool_SRCS
	src/motool/motool.cpp
)

add_executable(motool ${motool_SRCS})
target_link_libraries(motool mach-o)

install(TARGETS dyld${SUFFIX} DESTINATION bin)

# If building on a 64bit system, only install these for 64bits
if (NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")
	install(TARGETS motool DESTINATION bin)
	install(FILES etc/dylib.conf DESTINATION /etc/darling)
	add_subdirectory(src/macbinary)
	add_subdirectory(src/libmacarchive)
	add_subdirectory(src/crash)
endif(NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")

install(TARGETS mach-o DESTINATION "lib${SUFFIX}/darling")

if (WITH_TESTS)
	add_subdirectory(tests)
endif (WITH_TESTS)

add_subdirectory(src/libSystem)
add_subdirectory(src/Cocoa)
add_subdirectory(src/libobjcdarwin)
add_subdirectory(src/CoreFoundation)
add_subdirectory(src/libncurses)
add_subdirectory(src/CoreSecurity)
add_subdirectory(src/CoreServices)
add_subdirectory(src/IOKit)
add_subdirectory(src/libcrashhandler)
add_subdirectory(src/thin)
# Not ready yet
# add_subdirectory(src/cxx)

add_dependencies(dyld${SUFFIX} mach-o)
add_dependencies(System.B.dylib genfuncmap)
add_dependencies(Cocoa crashhandler)


add_subdirectory(src/AGL)
add_subdirectory(src/CoreAudio)
add_subdirectory(src/AudioToolbox)
add_subdirectory(src/AudioUnit)
add_subdirectory(src/QuickTime)
add_subdirectory(src/OpenGL)
add_subdirectory(src/CoreVideo)
