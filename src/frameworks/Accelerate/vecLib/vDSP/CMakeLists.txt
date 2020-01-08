project(vDSP)

set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libvDSP.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "671.250.4")

add_darling_library(vDSP SHARED
    src/vDSP.c
)
make_fat(vDSP)
target_link_libraries(vDSP system)
install(TARGETS vDSP DESTINATION libexec/darling/usr/lib)

set_property(TARGET vDSP PROPERTY DYLIB_INSTALL_NAME ${DYLIB_INSTALL_NAME})
set_property(TARGET vDSP PROPERTY DYLIB_BUILD_NAME libvDSP.dylib)
install(TARGETS vDSP DESTINATION libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A)
InstallSymlink(
	Versions/A/libvDSP.dylib
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/libvDSP.dylib
)
