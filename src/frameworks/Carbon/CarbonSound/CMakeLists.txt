project(CarbonSound)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(CarbonSound
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/CarbonSound"
)

add_framework(CarbonSound
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/SoundManager.cpp

	DEPENDENCIES
		CoreAudio
		AudioToolbox
		CoreFoundation
		system
		cxx
)
