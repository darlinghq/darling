project(system_dnssd)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_dnssd.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1096.60.2")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_darling_library(system_dnssd src/system_dnssd.c)
make_fat(system_dnssd)
target_link_libraries(system_dnssd PRIVATE system_c system_dyld)

install(TARGETS system_dnssd DESTINATION libexec/darling/usr/lib/system)
