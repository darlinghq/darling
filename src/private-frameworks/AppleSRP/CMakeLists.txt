project(AppleSRP)

# AppleSRP exports the API of the Stanford SRP library, which comes from the
# stanford-srp submodule. The library is built on the SSL library's bignum and
# SHA-1 (config.h selects them); its own SHA-1 implementation (t_sha.c) is not
# compiled in that configuration. include/AppleSRP holds links to the library's
# public headers.
set(SRP_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/external/stanford-srp/libsrp")

remove_sdk_framework(AppleSRP
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AppleSRP
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AppleSRP"
    PRIVATE
)

set(libsrp_sources
    ${SRP_SOURCE_DIR}/srp.c
    ${SRP_SOURCE_DIR}/srp6_client.c
    ${SRP_SOURCE_DIR}/srp6_server.c
    ${SRP_SOURCE_DIR}/rfc2945_client.c
    ${SRP_SOURCE_DIR}/rfc2945_server.c
    ${SRP_SOURCE_DIR}/t_math.c
    ${SRP_SOURCE_DIR}/cstr.c
    ${SRP_SOURCE_DIR}/t_conv.c
    ${SRP_SOURCE_DIR}/t_misc.c
    ${SRP_SOURCE_DIR}/t_truerand.c
    ${SRP_SOURCE_DIR}/t_conf.c
    ${SRP_SOURCE_DIR}/t_read.c
    ${SRP_SOURCE_DIR}/t_pw.c
)

set_source_files_properties(${libsrp_sources} PROPERTIES COMPILE_FLAGS "-std=gnu99 -DHAVE_CONFIG_H")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SRP_SOURCE_DIR}
)

add_framework(AppleSRP
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        ${libsrp_sources}

    LINK_FLAGS
        "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/AppleSRP.exp"

    DEPENDENCIES
        system
        crypto44
)
