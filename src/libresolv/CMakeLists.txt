project(libresolv)

cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/libdispatch/private)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dnsinfo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks -nostdinc -DUSE__RES_9")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

# Hide warnings
add_definitions(
	-Wno-tautological-pointer-compare
	-Wno-absolute-value
	-Wno-private-extern
)

set(resolv_sources
	base64.c
	dns_async.c
	dns.c
	dns_util.c
	dst_api.c
	dst_hmac_link.c
	dst_support.c
	ns_date.c
	ns_name.c
	ns_netint.c
	ns_parse.c
	ns_print.c
	ns_samedomain.c
	ns_sign.c
	ns_ttl.c
	ns_verify.c
	res_comp.c
	res_data.c
	res_debug.c
	res_findzonecut.c
	res_init.c
	res_mkquery.c
	res_mkupdate.c
	res_query.c
	res_send.c
	res_sendsigned.c
	res_update.c
	dnsinfo/dnsinfo.c
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(DYLIB_INSTALL_NAME "/usr/lib/libresolv.9.dylib")

add_circular(resolv-darwin FAT
	SOURCES
		${resolv_sources}
	SIBLINGS
		system_info
	DEPENDENCIES
		system_pthread
		system_malloc
		system_c
		system_kernel
		system_dnssd
		system_notify
		system_dyld
)
set_target_properties(resolv-darwin PROPERTIES OUTPUT_NAME "resolv.9")

InstallSymlink("libresolv.9.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libresolv.dylib")
install(TARGETS resolv-darwin DESTINATION libexec/darling/usr/lib)

