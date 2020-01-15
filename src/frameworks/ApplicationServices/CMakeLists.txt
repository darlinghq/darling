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

set(CoreGraphics_BUILD ${CMAKE_BINARY_DIR}/src/external/cocotron/CoreGraphics/CoreGraphics)
set(CoreText_BUILD ${CMAKE_BINARY_DIR}/src/external/cocotron/CoreText/CoreText)
set(CoreServices_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/CoreServices/CoreServices)
set(ImageIO_BUILD ${CMAKE_BINARY_DIR}/src/ImageIO/ImageIO)
set(ColorSync_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/ColorSync/ColorSync)
set(ATS_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/ATS/ATS)
set(ColorSyncLegacy_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/ColorSyncLegacy/ColorSyncLegacy)
set(HIServices_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/HIServices/HIServices)
set(LangAnalysis_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/LangAnalysis/LangAnalysis)
set(PrintCore_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/PrintCore/PrintCore)
set(QD_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/QD/QD)
set(SpeechSynthesis_BUILD ${CMAKE_BINARY_DIR}/src/frameworks/ApplicationServices/SpeechSynthesis/SpeechSynthesis)

reexport(ApplicationServices CoreGraphics ${CoreGraphics_BUILD})
reexport(ApplicationServices CoreText ${CoreText_BUILD})
reexport(ApplicationServices ImageIO ${ImageIO_BUILD})
reexport(ApplicationServices ColorSync ${ColorSync_BUILD})
reexport(ApplicationServices ATS ${ATS_BUILD})
reexport(ApplicationServices ColorSyncLegacy ${ColorSyncLegacy_BUILD})
reexport(ApplicationServices CoreServices ${CoreServices_BUILD})
reexport(ApplicationServices HIServices ${HIServices_BUILD})
reexport(ApplicationServices LangAnalysis ${LangAnalysis_BUILD})
reexport(ApplicationServices PrintCore ${PrintCore_BUILD})
reexport(ApplicationServices QD ${QD_BUILD})
reexport(ApplicationServices SpeechSynthesis ${SpeechSynthesis_BUILD})

InstallSymlink(../../../../ColorSync.framework
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/ColorSync.framework)

InstallSymlink(../../../../CoreGraphics.framework
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/CoreGraphics.framework)

InstallSymlink(../../../../CoreText.framework
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/CoreText.framework)

InstallSymlink(../../../../ImageIO.framework
	${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/ImageIO.framework)
