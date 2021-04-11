project(libc-x86_64)

cmake_minimum_required(VERSION 3.10)
enable_language(C ASM)

add_definitions(-DPRIVATE)

#set(CMAKE_C_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../../duct/include/commpage.h -include ${DARLING_TOP_DIRECTORY}/platform-include/sys/time.h")

if (TARGET_x86_64)
	set(x86_64_sources
		gen/mcount.S
	)
endif (TARGET_x86_64)

add_library(libc-x86_64 OBJECT ${x86_64_sources})

add_library(libc-hwplat_dyld OBJECT
	gen/mcount.S
)

set_target_properties(libc-hwplat_dyld PROPERTIES COMPILE_FLAGS "-DVARIANT_DYLD")

