project(AppleSRP)

# libsrp/ and include/AppleSRP/ are the Stanford SRP library that AppleSRP exports
# (github.com/secure-remote-password/stanford-srp @ 587900d). Only change: t_misc.c no longer
# creates a predictable /tmp file while seeding, which LibreSSL's RAND_bytes does not need.

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
    libsrp/srp.c
    libsrp/srp6_client.c
    libsrp/srp6_server.c
    libsrp/rfc2945_client.c
    libsrp/rfc2945_server.c
    libsrp/t_math.c
    libsrp/cstr.c
    libsrp/t_conv.c
    libsrp/t_misc.c
    libsrp/t_truerand.c
    libsrp/t_sha.c
    libsrp/t_conf.c
    libsrp/t_read.c
    libsrp/t_pw.c
)

set_source_files_properties(${libsrp_sources} PROPERTIES COMPILE_FLAGS "-std=gnu99 -DHAVE_CONFIG_H")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AppleSRP
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
