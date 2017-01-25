project(duct)

cmake_minimum_required(VERSION 2.4.0)

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__ -DPRIVATE)
add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,-bind_at_load -Wl,-flat_namespace -Wl,-undefined,suppress")

SET(duct_sources
	commpage.c libnotify.c numcpus.c
	CRGetCrashLogMessage.c acl.c bootstrap.c dns_sd.c
	sa_dst_compare.c asl.c)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_duct.dylib")
add_darling_library(system_duct SHARED ${duct_sources})
make_fat(system_duct)

set(CMAKE_AR "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/x86_64-apple-darwin11-ar")
set(CMAKE_RANLIB "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ranlib")
add_library(system_duct_static STATIC ${duct_sources})
add_dependencies(system_duct_static ranlib x86_64-apple-darwin11-ar)

install(TARGETS system_duct DESTINATION libexec/darling/usr/lib/system)
