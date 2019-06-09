project(ApplicationServices)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "48.0.0")

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
# reexport ATS
# reexport ColorSyncLegacy
reexport(ApplicationServices CoreServices)
# reexport HIServices
# reexport LangAnalysis
# reexport PrintCore
# reexport QD
# reexport SpeechSynthesis
