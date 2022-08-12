project(ImageCapture)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "5.0.0")

set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(ImageCapture
	BASE_PATH ${carbon_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "include/ImageCapture"
)

add_framework(ImageCapture
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "Carbon"

	SOURCES
		src/ImageCapture.m

	DEPENDENCIES
		system
		objc
		Foundation
)
