project(libgmalloc)

include_directories(
	${CMAKE_SOURCE_DIR}/src/dyld/include
)

set(libgmalloc_sources
	gmalloc.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/libgmalloc.dylib")
add_darling_library(gmalloc ${libgmalloc_sources})
target_link_libraries(gmalloc PRIVATE system)

install(TARGETS gmalloc DESTINATION libexec/darling/usr/lib)
