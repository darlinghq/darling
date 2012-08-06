project(dyld)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libmach-o)


set(dyld_SRCS
	FileMap.cpp
	ld-mac.cc
	log.cc
)

add_executable(dyld ${dyld_SRCS})

target_link_libraries(dyld -ldl -lpthread)

set(fatmacho-exract_SRCS
	extract.cpp
)

add_executable(fatmacho-exract ${fatmacho-exract_SRCS})

target_link_libraries(fatmacho-exract -ldl -lpthread mach-o)

install(TARGETS dyld fatmacho-exract DESTINATION bin)
