set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(SearchKit
	BASE_PATH ${coreservices_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "../../include/SearchKit"
)

add_framework(SearchKit
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "CoreServices"

	SOURCES
		empty.c

	DEPENDENCIES
		CoreFoundation
		system
)
