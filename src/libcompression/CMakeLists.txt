project(compression)

set(DYLIB_INSTALL_NAME "/usr/lib/libcompression.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(compression
	src/compression.c
)
make_fat(compression)
target_link_libraries(compression system)

install(TARGETS compression DESTINATION libexec/darling/usr/lib)
