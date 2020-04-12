add_framework(SharedFileList
	FAT
	CURRENT_VERSION
	VERSION "A"
	PARENT "CoreServices"
	SOURCES
		empty.c
	DEPENDENCIES
		CoreFoundation
		system
)
