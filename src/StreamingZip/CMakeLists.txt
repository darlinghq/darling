project(StreamingZip)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(SZ_C
	src/functions.c
	src/globals.c
)
set(SZ_OBJC
	src/StreamingUnzipper.m
	src/StreamingUnzipState.m
	src/SZExtractorInternalDelegate.m
	src/SZExtractor.m
)

add_framework(StreamingZip
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${SZ_C}
		${SZ_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
