add_framework(OSServices
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
