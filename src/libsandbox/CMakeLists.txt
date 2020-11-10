project(sandbox_not_system)

set(DYLIB_INSTALL_NAME "/usr/lib/libsandbox.1.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(sandbox_not_system SHARED
    src/sandbox.c
)
set_target_properties(sandbox_not_system PROPERTIES OUTPUT_NAME sandbox.1)
make_fat(sandbox_not_system)
target_link_libraries(sandbox_not_system system)
install(TARGETS sandbox_not_system DESTINATION libexec/darling/usr/lib)
InstallSymlink(libsandbox.1.dylib ${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libsandbox.dylib)
