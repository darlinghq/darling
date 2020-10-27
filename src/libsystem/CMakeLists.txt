project(darling-libsystem_init)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,-bind_at_load")

add_definitions(-DHAVE_STDINT_H=1 -DHAVE_SYSTEM_CORESERVICES)

# Hide warnings
add_definitions(
	-w
	-nostdinc
)

set(libsystem_sources
	init.c
	dummy.c
	CompatibilityHacks.c
)

set_source_files_properties(init.c PROPERTIES
	COMPILE_FLAGS " -DPRIVATE=1"
)

set(DYLIB_INSTALL_NAME "/usr/lib/libSystem.B.dylib")
set(DYLIB_CURRENT_VERSION "1281.0.0")
add_circular(system FAT
	SOURCES
		${libsystem_sources}

	SIBLINGS

	system_malloc
)

target_link_libraries(system PRIVATE
	sandbox
	system_quarantine
	removefile
	system_copyfile
	system_coreservices
	system_coretls
	system_c
	system_kernel
	system_trace
	keymgr

	# workaround a CMake/ld link order bug
	#
	# for some reason, Apple's linker (which we build and use) decided that if system_malloc appears
	# after system_m, it won't emit LC_REEXPORT_LIBRARY for system_malloc. instead, it emits LC_LOAD_LIBRARY,
	# which kind of borks the System umbrella library and defeats the point of including it
	#
	# to add to that, CMake decided that, after adding system_darwin, it would no longer place system_malloc
	# before system_m like we had specified here
	#
	# so, as a workaround, we add this order explicitly and then add dependencies on the targets
	#
	# the `-Wl,-warn_commons` is just to fool CMake into thinking this is a link flag
	"-Wl,-warn_commons ../libmalloc/libsystem_malloc.dylib ../libm/libsystem_m.dylib"

	system_info
	system_notify
	libdispatch_shared
	launch
	compiler_rt
	unwind
	system_dyld
	macho
	system_pthread
	platform
	system_asl
	system_duct
	system_blocks
	xpc
	corecrypto
	commonCrypto
	system_dnssd
	system_networkextension
	system_darwin
	libcache
)

add_dependencies(system system_malloc system_m)

set_target_properties(system PROPERTIES OUTPUT_NAME "System.B")
set_property(TARGET system APPEND_STRING PROPERTY LINK_FLAGS
	" -sub_library libsystem_malloc \
	-sub_library libsystem_c \
	-sub_library libsystem_kernel \
	-sub_library libkeymgr \
	-sub_library libsystem_m \
	-sub_library libsystem_info \
	-sub_library libsystem_notify \
	-sub_library libquarantine \
	-sub_library libsystem_blocks \
	-sub_library libsystem_duct \
	-sub_library libsystem_pthread \
	-sub_library libsystem_platform \
	-sub_library libsystem_trace \
	-sub_library libdispatch \
	-sub_library liblaunch \
	-sub_library libremovefile \
	-sub_library libcopyfile \
	-sub_library libunwind \
	-sub_library libdyld \
	-sub_library libcompiler_rt \
	-sub_library libmacho \
	-sub_library libcommonCrypto \
	-sub_library libsystem_sandbox \
	-sub_library libsystem_coreservices \
	-sub_library libsystem_coretls \
	-sub_library libsystem_asl \
	-sub_library libxpc \
	-sub_library libcorecrypto \
	-sub_library libsystem_dnssd \
	-sub_library libsystem_networkextension \
	-sub_library libsystem_darwin \
	-sub_library libcache")

install(TARGETS system DESTINATION libexec/darling/usr/lib)

InstallSymlink("libSystem.B.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libSystem.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libc.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libdbm.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libdl.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libinfo.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libdl.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libm.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libmx.A.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libmx.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libpoll.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libproc.dylib")
InstallSymlink("libSystem.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libpthread.dylib")
