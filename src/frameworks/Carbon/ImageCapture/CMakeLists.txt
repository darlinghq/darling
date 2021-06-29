project(ImageCapture)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "5.0.0")

add_framework(ImageCapture
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "Carbon"

	SOURCES
		src/ImageCapture.m

	DEPENDENCIES
		system
		objc
		Foundation
)
