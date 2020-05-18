project(libc-i386)

cmake_minimum_required(VERSION 2.8.0)
enable_language(C ASM)

#set(CMAKE_C_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../../duct/include/commpage.h -include ${DARLING_TOP_DIRECTORY}/platform-include/sys/time.h")

add_definitions(-DPRIVATE)

set(i386_sources
	gen/mcount.S
)

add_library(libc-i386 OBJECT ${i386_sources})


