
set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(Metadata
	BASE_PATH ${coreservices_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "../../include/Metadata"
)

add_framework(Metadata
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "CoreServices"

	SOURCES
		MDQuery.c
		MDItem.c

	DEPENDENCIES
		CoreFoundation
		system
)
