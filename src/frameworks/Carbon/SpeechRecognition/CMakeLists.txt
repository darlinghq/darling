project(SpeechRecognition)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(SpeechRecognition
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/SpeechRecognition"
)

add_framework(SpeechRecognition
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/SpeechRecognition.c

	DEPENDENCIES
		system
)
