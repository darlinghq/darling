project(dyld-bin)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Ttext-segment,0x400000 -Wl,-Tbss,0x410000 -Wl,-Tdata,0x420000")
add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" -D_GNU_SOURCE -DMLDR_BUILD)

add_executable(darling darling.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(mldr_sources
	mldr.c
	threads.c
	gdb.c
	commpage.c
	elfcalls.c
)
add_executable(mldr ${mldr_sources})
target_link_libraries(mldr pthread dl)

add_executable(mldr32 ${mldr_sources})
target_link_libraries(mldr32 pthread dl)
set_target_properties(mldr32
	PROPERTIES
		COMPILE_FLAGS "-m32"
		LINK_FLAGS "-m32"
)

add_executable(wrapgen wrapgen/wrapgen.cpp)
target_link_libraries(wrapgen dl)

install(TARGETS mldr mldr32 DESTINATION libexec/darling/bin)
install(TARGETS darling DESTINATION bin
    PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
    SETUID)

include(setcap)
setcap(libexec/darling/bin/mldr cap_sys_rawio+ep)
setcap(libexec/darling/bin/mldr32 cap_sys_rawio+ep)

