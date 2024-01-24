project(dyld-bin)

cmake_minimum_required(VERSION 3.13)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)

enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -ggdb -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Ttext-segment,0x400000 -Wl,-Tbss,0x410000 -Wl,-Tdata,0x420000")
add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" -D_GNU_SOURCE -DMLDR_BUILD)

if(TARGET_ARM64)
    add_definitions(-D__arm64__)
endif(TARGET_ARM64)

add_executable(darling darling.c)

target_link_libraries(darling -lutil)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/src/external/darlingserver/include ${CMAKE_SOURCE_DIR}/src/external/darlingserver/include)

install(TARGETS darling DESTINATION bin
    PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
    SETUID)

add_executable(rtsig rtsig.c)
add_custom_command(OUTPUT rtsig.h DEPENDS rtsig COMMAND ${CMAKE_CURRENT_BINARY_DIR}/rtsig rtsig.h COMMENT "Determining available RT signals")
add_custom_target(rtsig_h DEPENDS rtsig.h)

add_subdirectory(mldr)
