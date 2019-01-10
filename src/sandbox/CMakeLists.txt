project(libsandbox)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_sandbox.dylib")
add_darling_library(sandbox SHARED sandbox.c)
target_link_libraries(sandbox PRIVATE system_c system_kernel system_dyld system_malloc)
set_target_properties(sandbox PROPERTIES OUTPUT_NAME "system_sandbox")
make_fat(sandbox)

install(TARGETS sandbox DESTINATION libexec/darling/usr/lib/system)
