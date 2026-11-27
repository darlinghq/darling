project(System)

include(InstallSymlink)

install(FILES Info.plist
    DESTINATION libexec/darling/System/Library/Frameworks/System.framework/Versions/B/Resources
)

InstallSymlink(../../../../../../usr/lib/libSystem.B.dylib "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/System.framework/Versions/B/System")
InstallSymlink(B "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/System.framework/Versions/Current")
InstallSymlink(Versions/Current/System "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/System.framework/System")
InstallSymlink(Versions/Current/Resources "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/System.framework/Resources")
