project(BNNS)

set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libBNNS.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(BNNS SHARED
    src/BNNS.c
)
make_fat(BNNS)
target_link_libraries(BNNS system)
install(TARGETS BNNS DESTINATION libexec/darling/usr/lib)

set_property(TARGET BNNS PROPERTY DYLIB_INSTALL_NAME ${DYLIB_INSTALL_NAME})
set_property(TARGET BNNS PROPERTY DYLIB_BUILD_NAME libBNNS.dylib)
install(TARGETS BNNS DESTINATION libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A)
InstallSymlink(
	Versions/A/libBNNS.dylib
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/libBNNS.dylib
)
