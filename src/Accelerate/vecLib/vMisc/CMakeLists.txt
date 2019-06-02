project(libvMisc)

set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libvMisc.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "671.250.4")

add_darling_library(vMisc SHARED
    src/libvMisc.c
)
set_property(TARGET vMisc PROPERTY DYLIB_INSTALL_NAME ${DYLIB_INSTALL_NAME})
set_property(TARGET vMisc PROPERTY DYLIB_BUILD_NAME libvMisc.dylib)
make_fat(vMisc)
target_link_libraries(vMisc system)
install(TARGETS vMisc DESTINATION libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A)
InstallSymlink(
	Versions/A/libvMisc.dylib
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/libvMisc.dylib
)
