project(libLaunchServicesSupport)

include(darling_lib)

set(DYLIB_INSTALL_NAME "/usr/lib/libLaunchServicesSupport.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(LaunchServicesSupport SHARED
	src/LaunchServicesSupport.c
)
make_fat(LaunchServicesSupport)
target_link_libraries(LaunchServicesSupport system CoreFoundation)
set_target_properties(LaunchServicesSupport PROPERTIES LINKER_LANGUAGE C OUTPUT_NAME "LaunchServicesSupport")

install(TARGETS LaunchServicesSupport DESTINATION libexec/darling/usr/lib)
