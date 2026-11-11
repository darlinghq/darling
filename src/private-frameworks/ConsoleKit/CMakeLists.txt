project(ConsoleKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

# The framework exists only for 64-bit processes on macOS, so no 32-bit slice is built.
set(DARLING_LIB_64BIT_ONLY TRUE)

add_framework(ConsoleKit
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		src/ConsoleKit.m
		src/CSKAppKitAdditions.m
		src/CSKDevice.m
		src/CSKDirectoryObserver.m
		src/CSKFileSystem.m
		src/CSKFilter.m
		src/CSKHelpers.m
		src/CSKMessage.m
		src/CSKStreamArchiveSource.m
		src/CSKStreamASLSource.m
		src/CSKStreamViewController.m
		src/CSKTableColumnLayout.m
		src/CSKTokenFieldController.m

	DEPENDENCIES
		system
		objc
		Foundation
		AppKit
		CoreServices
)
target_compile_options(ConsoleKit PRIVATE -fobjc-arc)
target_include_directories(ConsoleKit PRIVATE include)
