project(QD)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(QD
    FAT
    CURRENT_VERSION
    VERSION "A"
    PARENT ApplicationServices

    SOURCES
    	src/QD.c

    DEPENDENCIES
	CoreFoundation
        system
)
