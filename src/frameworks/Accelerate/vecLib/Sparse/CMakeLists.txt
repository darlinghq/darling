project(Sparse)

set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libSparse.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(Sparse SHARED
    src/Sparse.c
)
make_fat(Sparse)
target_link_libraries(Sparse system)
install(TARGETS Sparse DESTINATION libexec/darling/usr/lib)

set_property(TARGET Sparse PROPERTY DYLIB_INSTALL_NAME ${DYLIB_INSTALL_NAME})
set_property(TARGET Sparse PROPERTY DYLIB_BUILD_NAME libSparse.dylib)
install(TARGETS Sparse DESTINATION libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A)
InstallSymlink(
	Versions/A/libSparse.dylib
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/libSparse.dylib
)
