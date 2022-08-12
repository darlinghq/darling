
set(SUBFRAMEWORK_VERSION "A")

generate_sdk_subframework(DictionaryServices
	BASE_PATH ${coreservices_sdk_path}
	VERSION ${SUBFRAMEWORK_VERSION}
	HEADER "../../include/DictionaryServices"
)

add_framework(DictionaryServices
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
