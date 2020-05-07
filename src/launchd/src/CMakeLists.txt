project(launchd)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(
	-w
	-nostdinc
)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../liblaunch
	${CMAKE_SOURCE_DIR}/src/external/libxpc
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks -include ${CMAKE_SOURCE_DIR}/kernel-include/bsd/sys/fileport.h")

mig(job.defs)
mig(helper.defs)
mig(internal.defs)
mig(job_forward.defs)
mig(job_reply.defs)
mig(mach_exc.defs)
mig(notify.defs)
#mig(job_types.defs)
#mig(protocol_jobmgr.defs)

# The xpc_domain_server definition seems to be wrong,
# but it's hard to determine the right solution.
add_definitions(-DPRIVATE -Dxpc_domain_server=job_server)

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
	${CMAKE_CURRENT_BINARY_DIR}/internalServer.c
	${CMAKE_CURRENT_BINARY_DIR}/internalUser.c
	${CMAKE_CURRENT_BINARY_DIR}/notifyServer.c
	${CMAKE_CURRENT_BINARY_DIR}/job_forwardUser.c
	${CMAKE_CURRENT_BINARY_DIR}/job_replyUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach_excServer.c
	#${CMAKE_CURRENT_BINARY_DIR}/job_typesUser.c
)

include(darling_exe)
add_darling_executable(launchd ${launchd_sources})
target_link_libraries(launchd bsm.0)

install(TARGETS launchd DESTINATION libexec/darling/sbin)

