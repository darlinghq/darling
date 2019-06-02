project(Quadrature)

set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libQuadrature.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(Quadrature SHARED
    src/Quadrature.c
)
make_fat(Quadrature)
target_link_libraries(Quadrature system)
install(TARGETS Quadrature DESTINATION libexec/darling/usr/lib)

set_property(TARGET Quadrature PROPERTY DYLIB_INSTALL_NAME ${DYLIB_INSTALL_NAME})
set_property(TARGET Quadrature PROPERTY DYLIB_BUILD_NAME libQuadrature.dylib)
install(TARGETS Quadrature DESTINATION libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A)
InstallSymlink(
	Versions/A/libQuadrature.dylib
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/libQuadrature.dylib
)
