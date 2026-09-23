project(ConsoleKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(ConsoleKit
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		src/ConsoleKit.m
		src/CSKAppKitAdditions.m
		src/CSKDevice.m
		src/CSKFileSystem.m
		src/CSKHelpers.m
		src/CSKMessage.m
		src/CSKStreamArchiveSource.m
		src/CSKStreamASLSource.m

	DEPENDENCIES
		system
		objc
		Foundation
		AppKit
)
target_compile_options(ConsoleKit PRIVATE -fobjc-arc)
target_include_directories(ConsoleKit PRIVATE include)
