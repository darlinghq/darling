project(MobileGestalt)

set(DYLIB_INSTALL_NAME "/usr/lib/libMobileGestalt.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(MobileGestalt SHARED
    src/MobileGestalt.m
)
make_fat(MobileGestalt)
target_link_libraries(MobileGestalt system objc Foundation)
install(TARGETS MobileGestalt DESTINATION libexec/darling/usr/lib)
