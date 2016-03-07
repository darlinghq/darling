project(launchd)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/private)
include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../liblaunch/)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/darwin)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/gen)
include_directories(${DARLING_TOP_DIRECTORY}/src/libinfo/lookup.subproj)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fblocks -nostdinc -include ${DARLING_TOP_DIRECTORY}/kernel-include/sys/fileport.h -ggdb -O0")
set(CMAKE_EXE_LINKER_FLAGS "-nodefaultlibs -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

mig(job.defs)
mig(helper.defs)
mig(internal.defs)
mig(job_forward.defs)
mig(job_reply.defs)
mig(mach_exc.defs)
mig(notify.defs)
#mig(job_types.defs)
#mig(protocol_jobmgr.defs)

add_definitions(-DPRIVATE)

set(launchd_sources
	core.c
	ipc.c
	kill2.c
	ktrace.c
	launchd.c
	log.c
	runtime.c
	${CMAKE_CURRENT_BINARY_DIR}/jobServer.c
	${CMAKE_CURRENT_BINARY_DIR}/helperUser.c
	${CMAKE_CURRENT_BINARY_DIR}/internalUser.c
	${CMAKE_CURRENT_BINARY_DIR}/notifyServer.c
	${CMAKE_CURRENT_BINARY_DIR}/job_forwardUser.c
	${CMAKE_CURRENT_BINARY_DIR}/job_replyUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach_excServer.c
	#${CMAKE_CURRENT_BINARY_DIR}/job_typesUser.c
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include(darling_exe)
add_darling_executable(launchd ${launchd_sources})
target_link_libraries(launchd xpc)

install(TARGETS launchd DESTINATION libexec/darling/sbin)

