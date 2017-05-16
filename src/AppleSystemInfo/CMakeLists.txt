project(AppleSystemInfo)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

add_framework(AppleSystemInfo
	PRIVATE
	VERSION "A"
	CURRENT_VERSION
	SOURCES
		functions.c
	DEPENDENCIES
		system
)

