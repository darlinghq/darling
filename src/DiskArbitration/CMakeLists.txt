project(DiskArbitration)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

set(da_SRCS
	DADisk.c
	DADissenter.c
	DASession.c
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(DiskArbitration
	FAT CURRENT_VERSION
	VERSION "A"
	SOURCES
		${da_SRCS}
	DEPENDENCIES
		system
		CoreFoundation
)
