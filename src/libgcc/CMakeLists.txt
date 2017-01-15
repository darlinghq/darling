project(libgcc)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

set(libgcc_sources
	dummy.c
)

add_darling_library(gcc_s SHARED ${libgcc_sources})
target_link_libraries(gcc_s PRIVATE compiler_rt -sub_library libcompiler_rt)
set_target_properties(gcc_s PROPERTIES OUTPUT_NAME "gcc_s.1")

install(TARGETS gcc_s DESTINATION libexec/darling/usr/lib)
