project(tools)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w -ggdb -O0")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

include_directories(${DARLING_TOP_DIRECTORY}/src/external/corefoundation/Headers)
include_directories(${CMAKE_BINARY_DIR}/src/external/corefoundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/foundation/Headers)
include_directories(${DARLING_TOP_DIRECTORY}/src/CFF)

include(darling_exe)

if (NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")

    add_darling_executable(sw_vers sw_vers.c)

    install(TARGETS sw_vers DESTINATION libexec/darling/usr/bin)
    target_link_libraries(sw_vers CFF CFFExtra)

    install(PROGRAMS sudo DESTINATION libexec/darling/usr/bin
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
                GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

endif (NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")

