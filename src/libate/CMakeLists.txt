project(libate)

set(DYLIB_INSTALL_NAME "/usr/lib/libate.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(ate SHARED
	libate.c
)
make_fat(ate)
target_link_libraries(ate system)
install(TARGETS ate DESTINATION libexec/darling/usr/lib)
