project(LinearAlgebra)

set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/libLinearAlgebra.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(LinearAlgebra SHARED
    src/LinearAlgebra.m
    src/OS_la_object.m
)
make_fat(LinearAlgebra)
target_link_libraries(LinearAlgebra system objc Foundation)
install(TARGETS LinearAlgebra DESTINATION libexec/darling/usr/lib)

set_property(TARGET LinearAlgebra PROPERTY DYLIB_INSTALL_NAME ${DYLIB_INSTALL_NAME})
set_property(TARGET LinearAlgebra PROPERTY DYLIB_BUILD_NAME libLinearAlgebra.dylib)
install(TARGETS LinearAlgebra DESTINATION libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A)
InstallSymlink(
	Versions/A/libLinearAlgebra.dylib
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/libLinearAlgebra.dylib
)
