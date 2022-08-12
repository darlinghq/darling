set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(OSServices
	BASE_PATH ${coreservices_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "../../include/OSServices"
)

add_framework(OSServices
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
