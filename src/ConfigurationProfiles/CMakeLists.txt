project(ConfigurationProfiles)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

add_framework(ConfigurationProfiles
	PRIVATE
	VERSION "A"
	CURRENT_VERSION
	SOURCES
		functions.c
	DEPENDENCIES
		system
)

