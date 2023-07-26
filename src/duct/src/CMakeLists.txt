project(duct)

cmake_minimum_required(VERSION 3.13)

add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-DPRIVATE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,-bind_at_load")

SET(duct_sources
	commpage.c libnotify.c numcpus.c
	CRGetCrashLogMessage.c acl.c dns_sd.c
	sa_dst_compare.c os_log.c)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_duct.dylib")
add_circular(system_duct FAT
	SOURCES
		${duct_sources}
	SIBLINGS
		system_kernel
		system_dyld
)
#make_fat(system_duct)

add_darling_static_library(system_duct_static SOURCES ${duct_sources})
make_fat(system_duct_static)

install(TARGETS system_duct DESTINATION libexec/darling/usr/lib/system)
