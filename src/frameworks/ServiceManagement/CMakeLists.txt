project(ServiceManagement)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "972.70.1")

add_framework(ServiceManagement
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
	  ServiceManagement.c

	DEPENDENCIES
		system
		CoreFoundation
		Security
		CoreServices
)
