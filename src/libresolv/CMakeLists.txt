project(libresolv)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/libdispatch/private)
include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${DARLING_TOP_DIRECTORY}/src/libinfo/lookup.subproj/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dnsinfo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks -nostdinc -DUSE__RES_9")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,-flat_namespace -Wl,-undefined,suppress")

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

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(DYLIB_INSTALL_NAME "/usr/lib/libresolv.9.dylib")
add_darling_library(resolv-darwin SHARED ${resolv_sources})
set_target_properties(resolv-darwin PROPERTIES OUTPUT_NAME "resolv.9")
#target_link_libraries(resolv-darwin PRIVATE system)

install(TARGETS resolv-darwin DESTINATION libexec/darling/usr/lib)

