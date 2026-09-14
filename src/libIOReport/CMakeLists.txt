project(libIOReport)

include(darling_lib)

set(DYLIB_INSTALL_NAME "/usr/lib/libIOReport.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(IOReport SHARED
	src/IOReport.c
)
make_fat(IOReport)
target_link_libraries(IOReport system CoreFoundation)
set_target_properties(IOReport PROPERTIES LINKER_LANGUAGE C OUTPUT_NAME "IOReport")

install(TARGETS IOReport DESTINATION libexec/darling/usr/lib)
