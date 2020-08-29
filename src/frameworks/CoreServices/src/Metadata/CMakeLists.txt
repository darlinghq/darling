
add_framework(Metadata
FAT
CURRENT_VERSION
VERSION "A"
PARENT "CoreServices"
SOURCES
	MDQuery.c
	MDItem.c
DEPENDENCIES
	CoreFoundation
	system
)
