project(AppleSauce)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "0.0.0")

add_framework(AppleSauce
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		src/AppleSauce.cpp

	DEPENDENCIES
		system
		cxx
)
