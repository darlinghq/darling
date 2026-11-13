# Copyright (C) 2026 Lubos Dolezel
#
# Builds Swift SDK overlay dylibs (libswiftDarwin.dylib and friends) from source with the Swift toolchain
# found by swift_toolchain.cmake, cross-compiling for macOS: swiftc compiles the sources against Darling's
# source-tree SDK, and the object is linked by Darling's ld64 like any other Darling dylib.
#
# The slice built is the one this build produces for every other Darling library (APPLE_ARCH_64BIT: x86_64, or
# arm64 when TARGET_ARM64 is on).
#
# add_darling_swift_library(<Module>
#     SOURCES <swift sources>
#     [FLAGS <extra swiftc -frontend flags>]
#     [MODULES <overlay Modules this one imports; built by this function earlier>]
#     [OBJECTS <objects from add_darling_swift_object linked into the dylib>]
#     [LINK_FLAGS <extra linker flags>]
#     [NATIVE_SOURCES <C/C++/Objective-C sources linked into the same dylib>]
#     [NATIVE_FLAGS <compile flags for NATIVE_SOURCES>]
#     [LIBRARIES <extra link libraries/targets>]
#     [INSTALL_NAME <name>])
#
# creates the target swift<Module> (libswift<Module>.dylib, install name /usr/lib/swift/libswift<Module>.dylib)
# and the module <Module>.swiftmodule in ${DARLING_SWIFT_MODULE_DIR} for overlays that import it.
#
# Required variables: SWIFT_TOOLCHAIN_FOUND and friends (swift_toolchain.cmake), and
#   DARLING_SWIFT_STDLIB_MODULES  directory with <Name>.swiftmodule/<arch>-apple-macos.swiftinterface for the
#                                 Swift runtime the overlays run on
#   DARLING_SWIFT_RUNTIME_DIR     directory with the runtime's libswiftCore.dylib

include(CMakeParseArguments)

set(DARLING_SWIFT_DEPLOYMENT_TARGET "26.0" CACHE INTERNAL "macOS version the Swift overlays are built for")
set(DARLING_SWIFT_SDK "${DARLING_TOP_DIRECTORY}/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
# Foundation's headers include <AE/AE.h>, which the SDK ships as a subframework of CoreServices.
set(DARLING_SWIFT_SDK_SUBFRAMEWORKS -Xcc -iframework -Xcc "${DARLING_SWIFT_SDK}/System/Library/Frameworks/CoreServices.framework/Frameworks")
set(DARLING_SWIFT_MODULE_DIR "${CMAKE_BINARY_DIR}/swift-overlay-modules")
set(DARLING_SWIFT_MODULE_CACHE "${CMAKE_BINARY_DIR}/swift-module-cache")

# The toolchain's own resource directory carries Linux-only module maps (dispatch, os, CoreFoundation) that would
# clash with the SDK's, so swiftc gets a resource directory with just the compiler-internal parts.
function(_darling_swift_resource_dir out)
	set(dir "${CMAKE_BINARY_DIR}/swift-resource")
	file(MAKE_DIRECTORY "${dir}")
	foreach(entry shims apinotes)
		execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink "${SWIFT_TOOLCHAIN_USR}/lib/swift/${entry}" "${dir}/${entry}")
	endforeach()
	file(GLOB clang_dirs "${SWIFT_TOOLCHAIN_USR}/lib/clang/*")
	list(GET clang_dirs 0 clang_dir)
	execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink "${clang_dir}" "${dir}/clang")
	set(${out} "${dir}" PARENT_SCOPE)
endfunction()

# add_darling_swift_object(<Module> SOURCES <swift sources> [FLAGS ...] [MODULES ...])
#
# Compiles an implementation-detail Swift module that is not shipped as a dylib: no library evolution, no autolink
# entries. Sets <Module>_OBJECT in the caller's scope to the object file, to pass to add_darling_swift_library(OBJECTS),
# and emits <Module>.swiftmodule into ${DARLING_SWIFT_MODULE_DIR}.
function(add_darling_swift_object module)
	cmake_parse_arguments(SWIFTOBJ "" "" "SOURCES;FLAGS;MODULES" ${ARGN})
	if (NOT SWIFT_TOOLCHAIN_FOUND OR NOT BUILD_TARGET_64BIT)
		return()
	endif()
	_darling_swift_resource_dir(resource_dir)
	file(MAKE_DIRECTORY "${DARLING_SWIFT_MODULE_DIR}" "${DARLING_SWIFT_MODULE_CACHE}")
	set(obj "${CMAKE_CURRENT_BINARY_DIR}/${module}.swift.o")
	set(swiftmodule "${DARLING_SWIFT_MODULE_DIR}/${module}.swiftmodule")
	set(module_deps)
	foreach(dep IN LISTS SWIFTOBJ_MODULES)
		list(APPEND module_deps "${DARLING_SWIFT_MODULE_DIR}/${dep}.swiftmodule")
	endforeach()
	set(sources)
	foreach(src IN LISTS SWIFTOBJ_SOURCES)
		get_filename_component(src "${src}" ABSOLUTE)
		list(APPEND sources "${src}")
	endforeach()
	add_custom_command(
		OUTPUT "${obj}" "${swiftmodule}"
		COMMAND "${SWIFT_COMPILER}" -frontend -c ${sources}
			-target "${APPLE_ARCH_64BIT}-apple-macosx${DARLING_SWIFT_DEPLOYMENT_TARGET}"
			-sdk "${DARLING_SWIFT_SDK}" -resource-dir "${resource_dir}"
			-I "${DARLING_SWIFT_STDLIB_MODULES}" -I "${DARLING_SWIFT_MODULE_DIR}"
			-module-cache-path "${DARLING_SWIFT_MODULE_CACHE}"
			-swift-version 5 -module-name "${module}" -parse-as-library -O
			${DARLING_SWIFT_SDK_SUBFRAMEWORKS}
			${SWIFTOBJ_FLAGS}
			-emit-module-path "${swiftmodule}" -o "${obj}"
		DEPENDS ${sources} ${module_deps}
		COMMENT "Compiling the Swift module ${module} for ${APPLE_ARCH_64BIT}"
		VERBATIM
	)
	set_source_files_properties("${obj}" PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
	set(${module}_OBJECT "${obj}" PARENT_SCOPE)
endfunction()

function(add_darling_swift_library module)
	cmake_parse_arguments(SWIFTLIB "" "INSTALL_NAME" "SOURCES;FLAGS;MODULES;OBJECTS;LINK_FLAGS;NATIVE_SOURCES;NATIVE_FLAGS;LIBRARIES" ${ARGN})

	if (NOT SWIFT_TOOLCHAIN_FOUND)
		message(FATAL_ERROR "add_darling_swift_library(${module}) needs the Swift toolchain")
	endif()
	if (NOT BUILD_TARGET_64BIT)
		return()
	endif()
	if (NOT SWIFTLIB_SOURCES)
		message(FATAL_ERROR "add_darling_swift_library(${module}): no SOURCES")
	endif()
	if (NOT DARLING_SWIFT_STDLIB_MODULES OR NOT DARLING_SWIFT_RUNTIME_DIR)
		message(FATAL_ERROR "DARLING_SWIFT_STDLIB_MODULES and DARLING_SWIFT_RUNTIME_DIR must be set")
	endif()

	_darling_swift_resource_dir(resource_dir)
	file(MAKE_DIRECTORY "${DARLING_SWIFT_MODULE_DIR}" "${DARLING_SWIFT_MODULE_CACHE}")

	set(target "${APPLE_ARCH_64BIT}-apple-macosx${DARLING_SWIFT_DEPLOYMENT_TARGET}")
	set(obj "${CMAKE_CURRENT_BINARY_DIR}/swift${module}.swift.o")
	set(swiftmodule "${DARLING_SWIFT_MODULE_DIR}/${module}.swiftmodule")

	set(module_deps)
	set(module_libs)
	foreach(dep IN LISTS SWIFTLIB_MODULES)
		list(APPEND module_deps "${DARLING_SWIFT_MODULE_DIR}/${dep}.swiftmodule")
		# Modules from add_darling_swift_object have no dylib of their own.
		if (TARGET swift${dep})
			list(APPEND module_libs "swift${dep}")
		endif()
	endforeach()

	set(sources)
	foreach(src IN LISTS SWIFTLIB_SOURCES)
		get_filename_component(src "${src}" ABSOLUTE)
		list(APPEND sources "${src}")
	endforeach()

	# Library evolution matches Apple's overlays, so binaries built against the macOS SDK use the same access
	# patterns. The platform-neutral SwiftStdlib availability names are what the overlay sources use for the
	# stdlib's release versions.
	add_custom_command(
		OUTPUT "${obj}" "${swiftmodule}"
		COMMAND "${SWIFT_COMPILER}" -frontend -c ${sources}
			-target "${target}" -sdk "${DARLING_SWIFT_SDK}" -resource-dir "${resource_dir}"
			-I "${DARLING_SWIFT_STDLIB_MODULES}" -I "${DARLING_SWIFT_MODULE_DIR}"
			-module-cache-path "${DARLING_SWIFT_MODULE_CACHE}"
			-swift-version 5 -module-name "${module}" -module-link-name "swift${module}" -autolink-force-load
			-enable-library-evolution -parse-as-library -O
			-define-availability "SwiftStdlib 5.7:macOS 13.0"
			${DARLING_SWIFT_SDK_SUBFRAMEWORKS}
			${SWIFTLIB_FLAGS}
			-emit-module-path "${swiftmodule}" -o "${obj}"
		DEPENDS ${sources} ${module_deps}
		COMMENT "Compiling the Swift overlay ${module} for ${APPLE_ARCH_64BIT}"
		VERBATIM
	)
	set_source_files_properties("${obj}" PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)

	if (SWIFTLIB_INSTALL_NAME)
		set(DYLIB_INSTALL_NAME "${SWIFTLIB_INSTALL_NAME}")
	else()
		set(DYLIB_INSTALL_NAME "/usr/lib/swift/libswift${module}.dylib")
	endif()
	set(DARLING_LIB_64BIT_ONLY TRUE)

	add_darling_library(swift${module} SHARED "${obj}" ${SWIFTLIB_OBJECTS} ${SWIFTLIB_NATIVE_SOURCES})
	if (SWIFTLIB_LINK_FLAGS)
		list(JOIN SWIFTLIB_LINK_FLAGS " " extra_link_flags)
		set_property(TARGET swift${module} APPEND_STRING PROPERTY LINK_FLAGS " ${extra_link_flags}")
	endif()
	if (SWIFTLIB_NATIVE_SOURCES AND SWIFTLIB_NATIVE_FLAGS)
		list(JOIN SWIFTLIB_NATIVE_FLAGS " " native_flags)
		set_source_files_properties(${SWIFTLIB_NATIVE_SOURCES} PROPERTIES COMPILE_FLAGS "${native_flags}")
	endif()
	set_target_properties(swift${module} PROPERTIES
		LINKER_LANGUAGE C
		OUTPUT_NAME "swift${module}"
	)
	# The overlays are built for the macOS version they compile against: libswiftCoreFoundation hides CGFloat's
	# old CoreGraphics-mangled symbols from clients that target macOS before 12, and Darling's default deployment
	# target is older than that.
	set_property(TARGET swift${module} APPEND_STRING PROPERTY LINK_FLAGS
		" -mmacosx-version-min=${DARLING_SWIFT_DEPLOYMENT_TARGET}")
	# The objects' autolink entries name the runtime and the overlays this one imports.
	set_property(TARGET swift${module} APPEND_STRING PROPERTY LINK_FLAGS
		" -Wl,-L,${DARLING_SWIFT_RUNTIME_DIR} -Wl,-L,${CMAKE_CURRENT_BINARY_DIR}")
	target_link_libraries(swift${module} system objc "${DARLING_SWIFT_RUNTIME_DIR}/libswiftCore.dylib"
		${module_libs} ${SWIFTLIB_LIBRARIES})
	foreach(dep IN LISTS SWIFTLIB_MODULES)
		if (TARGET swift${dep})
			add_dependencies(swift${module} swift${dep})
		endif()
	endforeach()
	install(TARGETS swift${module} DESTINATION libexec/darling/usr/lib/swift)
endfunction()
