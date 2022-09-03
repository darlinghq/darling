project(AppleSauce)

remove_sdk_framework(AppleSauce
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "0.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AppleSauce
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AppleSauce"
    PRIVATE
)

add_framework(AppleSauce
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/AppleSauce.cpp

	DEPENDENCIES
		system
		cxx
)
