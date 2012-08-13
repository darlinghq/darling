project(darling)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/xnu)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/xnu/bsd)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libmach-o)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/util)

configure_file(config.h.in config.h)

set(mach-o_SRCS
	src/libmach-o/MachO.cpp
	src/libmach-o/FatMachO.cpp
	src/libmach-o/RebaseState.cpp
	src/libmach-o/BindState.cpp
	src/libmach-o/leb.cpp
	src/libmach-o/MachOImpl.cpp
	src/util/log.cc
)

add_library(mach-o SHARED ${mach-o_SRCS})
set_target_properties(mach-o PROPERTIES VERSION 1.0.0 SOVERSION 1.0)
set_target_properties(mach-o PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")
target_link_libraries(mach-o -ldl -lpthread)

set(dyld_SRCS
	src/dyld/FileMap.cpp
	src/dyld/ld-mac.cc
	src/util/log.cc
	src/util/trace.cpp
)

add_executable(dyld ${dyld_SRCS})

target_link_libraries(dyld -ldl -lpthread mach-o)

set(fatmacho-extract_SRCS
	src/dyld/extract.cpp
)

add_executable(fatmacho-extract ${fatmacho-extract_SRCS})

target_link_libraries(fatmacho-extract -ldl -lpthread mach-o)

install(TARGETS dyld fatmacho-extract DESTINATION bin)

add_subdirectory(src/libSystem src/libunwind-darwin)

