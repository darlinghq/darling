project(CFOpenDirectory)

remove_sdk_framework(CFOpenDirectory)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

get_path_preframework(opendirectory_developer_path)
append_path_sdk_subframework(${opendirectory_developer_path} opendirectory_developer_path OpenDirectory
    VERSION ${FRAMEWORK_VERSION}
)

generate_sdk_subframework(CFOpenDirectory
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CFOpenDirectory"
	BASE_PATH ${opendirectory_developer_path}
)

add_framework(CFOpenDirectory
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/CFOpenDirectory.c
		src/constants.c

	DEPENDENCIES
		system
		CoreFoundation
)
