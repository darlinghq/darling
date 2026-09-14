project(ScreenReader)

set(DYLIB_INSTALL_NAME "/usr/lib/libScreenReader.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(ScreenReader SHARED
    src/ScreenReader.c
)
make_fat(ScreenReader)
target_link_libraries(ScreenReader system)
install(TARGETS ScreenReader DESTINATION libexec/darling/usr/lib)
