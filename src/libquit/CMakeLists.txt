project(quit)

set(DYLIB_INSTALL_NAME "/usr/lib/libquit.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(quit SHARED
    src/quit.c
)
make_fat(quit)
target_link_libraries(quit system)
install(TARGETS quit DESTINATION libexec/darling/usr/lib)
