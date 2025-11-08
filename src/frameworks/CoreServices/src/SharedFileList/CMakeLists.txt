set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(SharedFileList
	BASE_PATH ${coreservices_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "../../include/SharedFileList"
)

add_framework(SharedFileList
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "CoreServices"

	SOURCES
		constants.c

	DEPENDENCIES
		CoreFoundation
		system
)
