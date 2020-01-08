project(ATS)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "236.0.0")

add_framework(ATS
    FAT
    CURRENT_VERSION
    VERSION "A"
    PARENT ApplicationServices

    SOURCES
    	src/ATS.c

    DEPENDENCIES
	CoreFoundation
        system
)
