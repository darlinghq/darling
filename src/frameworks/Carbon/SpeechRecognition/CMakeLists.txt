project(SpeechRecognition)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(SpeechRecognition
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "Carbon"

	SOURCES
		src/SpeechRecognition.c

	DEPENDENCIES
		system
)
