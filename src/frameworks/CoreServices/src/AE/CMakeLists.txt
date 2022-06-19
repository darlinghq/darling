
add_framework(AE
FAT
CURRENT_VERSION
VERSION "A"
PARENT "CoreServices"
SOURCES
	empty.c
	stub.c
DEPENDENCIES
	CoreFoundation
	system
)
