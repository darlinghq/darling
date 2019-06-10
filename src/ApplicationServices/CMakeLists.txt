project(ApplicationServices)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "48.0.0")

add_subdirectory(HIServices)
add_subdirectory(ATS)
add_subdirectory(QD)
add_subdirectory(ColorSyncLegacy)
add_subdirectory(LangAnalysis)
add_subdirectory(PrintCore)
add_subdirectory(SpeechSynthesis)

add_framework(ApplicationServices
	FAT CURRENT_VERSION
	VERSION "A"
	SOURCES
		ApplicationServices.c
	DEPENDENCIES
	system CoreServices CoreFoundation
)

reexport(ApplicationServices CoreGraphics)
reexport(ApplicationServices CoreText)
reexport(ApplicationServices ImageIO)
reexport(ApplicationServices ColorSync)
reexport(ApplicationServices ATS)
reexport(ApplicationServices ColorSyncLegacy)
reexport(ApplicationServices CoreServices)
reexport(ApplicationServices HIServices)
reexport(ApplicationServices LangAnalysis)
reexport(ApplicationServices PrintCore)
reexport(ApplicationServices QD)
reexport(ApplicationServices SpeechSynthesis)

InstallSymlink(../../../../ColorSync.framework
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/ColorSync.framework)

InstallSymlink(../../../../CoreGraphics.framework
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/CoreGraphics.framework)

InstallSymlink(../../../../CoreText.framework
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/CoreText.framework)

InstallSymlink(../../../../ImageIO.framework
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/ImageIO.framework)
