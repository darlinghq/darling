project(tools)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03")

include_directories(
	${CMAKE_SOURCE_DIR}/src/external/corefoundation
	${CMAKE_SOURCE_DIR}/src/external/libclosure
	${CMAKE_SOURCE_DIR}/src/launchd/liblaunch
)

add_darling_executable(sw_vers sw_vers.c)

install(TARGETS sw_vers DESTINATION libexec/darling/usr/bin)
target_link_libraries(sw_vers system CoreFoundation)

install(PROGRAMS sudo DESTINATION libexec/darling/usr/bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ
            GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
