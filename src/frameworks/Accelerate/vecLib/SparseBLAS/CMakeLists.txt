project(SparseBLAS)

set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libSparseBLAS.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(SparseBLAS SHARED
    src/SparseBLAS.c
)
make_fat(SparseBLAS)
target_link_libraries(SparseBLAS system)
install(TARGETS SparseBLAS DESTINATION libexec/darling/usr/lib)

set_property(TARGET SparseBLAS PROPERTY DYLIB_INSTALL_NAME ${DYLIB_INSTALL_NAME})
set_property(TARGET SparseBLAS PROPERTY DYLIB_BUILD_NAME libSparseBLAS.dylib)
install(TARGETS SparseBLAS DESTINATION libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A)
InstallSymlink(
	Versions/A/libSparseBLAS.dylib
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/libSparseBLAS.dylib
)
