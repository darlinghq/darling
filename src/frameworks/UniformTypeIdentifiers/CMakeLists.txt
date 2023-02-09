project(UniformTypeIdentifiers)

remove_sdk_framework(UniformTypeIdentifiers)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "709.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(UniformTypeIdentifiers
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/UniformTypeIdentifiers"
)


add_framework(UniformTypeIdentifiers
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/UTType.m
		src/_UTConstantType.m
		src/_UTCoreType.m
		src/_UTRuntimeConstantType.m
		src/_UTTaggedType.m
		src/NSURL+UTAdditions.m
		src/NSItemProvider+UTType.m
		src/_UTCoreType+UTRefCounting.m
		src/NSString+UTAdditions.m
		src/UniformTypeIdentifiers.m

	DEPENDENCIES
		system
		objc
		Foundation
)