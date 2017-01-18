project(dyld-bin)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -ggdb")
add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" -D_GNU_SOURCE -DMLDR_BUILD)

add_executable(darling darling.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(mldr_sources
	mldr.c
	threads.c
	gdb.c
)
add_executable(mldr ${mldr_sources})
target_link_libraries(mldr -lpthread -ldl)

install(TARGETS darling DESTINATION bin)
install(TARGETS mldr DESTINATION libexec)
