set(SUBFRAMEWORK_VERSION "A")

remove_sdk_framework(AE)

generate_sdk_subframework(AE
	BASE_PATH ${coreservices_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "../../include/AE"
)

add_framework(AE
	FAT
	CURRENT_VERSION
	VERSION ${SUBFRAMEWORK_VERSION}
	PARENT "CoreServices"

	SOURCES
		empty.c
		stub.c

	DEPENDENCIES
		CoreFoundation
		system
)
