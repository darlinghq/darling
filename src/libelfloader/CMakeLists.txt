project(libelfloader)

set(elfloader_sources
	loader.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/darling/libelfloader.dylib")
add_darling_library(elfloader SHARED ${elfloader_sources})
target_link_libraries(elfloader system)
make_fat(elfloader)

install(TARGETS elfloader DESTINATION libexec/darling/usr/lib/darling)

