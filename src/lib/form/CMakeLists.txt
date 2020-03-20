project(form)

set(DYLIB_INSTALL_NAME "/usr/lib/libform.5.4.dylib")
set(DYLIB_COMPAT_VERSION "5.4.0")
set(DYLIB_CURRENT_VERSION "5.4.0")

add_darling_library(form SHARED
    src/form.c
)
make_fat(form)
target_link_libraries(form system)
set_target_properties(form PROPERTIES OUTPUT_NAME "form.5.4")
install(TARGETS form DESTINATION libexec/darling/usr/lib)
InstallSymlink(libform.5.4.dylib "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libform.dylib")
