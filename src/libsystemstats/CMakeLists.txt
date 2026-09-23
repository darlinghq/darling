project(libsystemstats)

include(darling_lib)

set(DYLIB_INSTALL_NAME "/usr/lib/libsystemstats.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(systemstats SHARED
	src/systemstats.c
)
make_fat(systemstats)
target_link_libraries(systemstats system)
set_target_properties(systemstats PROPERTIES LINKER_LANGUAGE C OUTPUT_NAME "systemstats")

install(TARGETS systemstats DESTINATION libexec/darling/usr/lib)
