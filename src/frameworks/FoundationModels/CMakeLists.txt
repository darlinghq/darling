project(FoundationModels)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

if (SWIFT_TOOLCHAIN_FOUND AND BUILD_TARGET_64BIT)
	set(DARLING_SWIFT_STDLIB_MODULES "${DARLING_TOP_DIRECTORY}/src/external/swift/stdlib-modules")
	set(DARLING_SWIFT_RUNTIME_DIR "${DARLING_TOP_DIRECTORY}/src/external/swift")

	add_darling_swift_library(FoundationModels
		FRAMEWORK
		SOURCES
			src/FoundationModels.swift
		MODULES
			ObjectiveC
			Darwin
		FLAGS
			-Xcc -fmodule-map-file=${DARLING_TOP_DIRECTORY}/src/external/swift/overlays/shims/overlay-shims.modulemap
		LINK_FLAGS
			-Wl,-L,${CMAKE_BINARY_DIR}/src/external/swift
	)

	install(FILES Info.plist
		DESTINATION libexec/darling/System/Library/Frameworks/FoundationModels.framework/Versions/A/Resources
	)
	InstallSymlink("Versions/Current/Resources"
		"${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/Frameworks/FoundationModels.framework/Resources"
	)
elseif (NOT SWIFT_TOOLCHAIN_FOUND)
	message(STATUS "Skipping FoundationModels.framework: no Swift toolchain")
endif()
