# Copyright (C) 2026 Lubos Dolezel
#
# Builds Swift SDK overlay dylibs (libswiftDarwin.dylib and friends) from source with the Swift toolchain
# found by swift_toolchain.cmake, cross-compiling for macOS: swiftc compiles the sources against Darling's
# source-tree SDK, and the object is linked by Darling's ld64 like any other Darling dylib.
#
# The overlays have the 64-bit slices of DARLING_ARCHS (the Swift runtime has no 32-bit slice). Each slice is
# compiled on its own, against that architecture's stdlib interfaces and with its own module directory
# (${DARLING_SWIFT_MODULE_DIR}/<arch>), and the objects of the slices are joined with lipo before the link.
#
# add_darling_swift_library(<Module>
#     SOURCES <swift sources>
#     [FLAGS <extra swiftc -frontend flags>]
#     [MODULES <overlay Modules this one imports; built by this function earlier>]
#     [WEAK_MODULES <overlay Modules that are imported like MODULES but whose dylib is linked weakly (the load
#                    command is LC_LOAD_WEAK_DYLIB), for dependencies that macOS also links weakly>]
#     [OBJECTS <objects from add_darling_swift_object linked into the dylib>]
#     [LINK_FLAGS <extra linker flags>]
#     [NATIVE_SOURCES <C/C++/Objective-C sources linked into the same dylib>]
#     [NATIVE_FLAGS <compile flags for NATIVE_SOURCES>]
#     [LIBRARIES <extra link libraries/targets>]
#     [INSTALL_NAME <name>]
#     [FRAMEWORK])
#
# creates the target swift<Module> (libswift<Module>.dylib, install name /usr/lib/swift/libswift<Module>.dylib)
# and the module <Module>.swiftmodule in ${DARLING_SWIFT_MODULE_DIR}/<arch> for overlays that import it.
#
# With FRAMEWORK the library is the binary of a framework instead (Combine): it is built as
# <Module>.framework/Versions/A/<Module> in the current binary directory, has the install name
# /System/Library/Frameworks/<Module>.framework/Versions/A/<Module> unless INSTALL_NAME is given, and is installed
# with the Versions/Current and top-level symlinks. The target keeps the name swift<Module>. The module's
# autolink entry names -lswift<Module>, so a libswift<Module>.dylib symlink to the binary is created next to it
# (the directory every overlay already searches with -L).
#
# Compilation is parallel. A module with at least DARLING_SWIFT_PARALLEL_MIN_SOURCES sources is compiled by the swiftc
# driver in batch mode (-j DARLING_SWIFT_JOBS): every source file gets its own object file and the files are compiled
# concurrently, instead of one single-threaded whole-module frontend job. The type-checking and SIL optimisation
# that dominate a whole-module build are not split by -num-threads (that only parallelises LLVM code generation), so
# per-file jobs are the only way to use more than one core. The price is that the optimiser sees one file at a time
# (about three times the CPU time and a slightly larger dylib). The module interface and the dylib's export table are
# unchanged. With DARLING_SWIFT_WMO=ON (packaging and CI builds), DARLING_SWIFT_JOBS=1 or for small modules the
# whole-module job is used.
#
# Required variables: SWIFT_TOOLCHAIN_FOUND and friends (swift_toolchain.cmake), and
#   DARLING_SWIFT_STDLIB_MODULES  directory with <Name>.swiftmodule/<arch>-apple-macos.swiftinterface for the
#                                 Swift runtime the overlays run on
#   DARLING_SWIFT_RUNTIME_DIR     directory with the runtime's libswiftCore.dylib

include(CMakeParseArguments)
include(InstallSymlink)

cmake_host_system_information(RESULT _darling_swift_cores QUERY NUMBER_OF_LOGICAL_CORES)
set(DARLING_SWIFT_JOBS "${_darling_swift_cores}" CACHE STRING
	"Number of parallel swiftc jobs for each Swift overlay module (1: single whole-module job)")
set(DARLING_SWIFT_PARALLEL_MIN_SOURCES "16" CACHE STRING
	"Modules with fewer sources than this are compiled as one whole-module job (the build time is negligible)")
option(DARLING_SWIFT_WMO "Compile every Swift overlay module as one whole-module job (slower build, best optimisation; for packaging and CI)" OFF)
if (CMAKE_GENERATOR MATCHES "Ninja" AND NOT CMAKE_VERSION VERSION_LESS 3.15)
	# The swiftc driver runs its own jobs, so at most one module is compiled at a time to avoid oversubscribing the CPUs
	# on top of Ninja's -j. (Make has no such pool: with make -jN the module jobs can overlap.)
	set_property(GLOBAL APPEND PROPERTY JOB_POOLS darling_swift=1)
	set(_DARLING_SWIFT_JOB_POOL JOB_POOL darling_swift)
else()
	set(_DARLING_SWIFT_JOB_POOL)
endif()
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

# The slices the overlays are built for
function(_darling_swift_archs out)
	set(${out} ${DARLING_ARCHS_64BIT} PARENT_SCOPE)
endfunction()

# _darling_swift_fat_objects(<name> <out var> <prefix> <arch>...)
#
# Joins the objects of the slices, given in the caller's variables <prefix>_<arch> (the same number of objects for
# every slice, in the same order), into fat objects with lipo. With one slice its objects are used as they are.
function(_darling_swift_fat_objects name out_var prefix)
	set(archs ${ARGN})
	list(LENGTH archs arch_count)
	list(GET archs 0 first_arch)
	if (arch_count EQUAL 1)
		set(${out_var} ${${prefix}_${first_arch}} PARENT_SCOPE)
		return()
	endif()
	set(fat_dir "${CMAKE_CURRENT_BINARY_DIR}/swift-objects/${name}.fat")
	file(MAKE_DIRECTORY "${fat_dir}")
	set(lipo "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/lipo")
	list(LENGTH ${prefix}_${first_arch} object_count)
	math(EXPR last "${object_count} - 1")
	set(fat_objects)
	foreach(index RANGE ${last})
		set(inputs)
		foreach(arch IN LISTS archs)
			list(GET ${prefix}_${arch} ${index} object)
			list(APPEND inputs "${object}")
		endforeach()
		set(fat "${fat_dir}/${index}.o")
		add_custom_command(
			OUTPUT "${fat}"
			COMMAND "${lipo}" -create ${inputs} -output "${fat}"
			DEPENDS ${inputs} lipo
			VERBATIM
		)
		list(APPEND fat_objects "${fat}")
	endforeach()
	set(${out_var} ${fat_objects} PARENT_SCOPE)
endfunction()

# _darling_swift_compile(<name> <out var for the objects>
#     OBJECT <object path used by the whole-module job> SOURCES <abs paths> COMMON <swiftc flags> MODULE <swiftmodule>
#     DEPENDS <files> COMMENT <text>)
#
# Adds the command that compiles the sources and emits the module. Whole-module: one object (OBJECT); parallel: one
# object per source in <binary dir>/swift-objects/<name>/, listed in the out variable. <name> is <Module>.<arch>.
function(_darling_swift_compile name out_var)
	cmake_parse_arguments(C "" "OBJECT;MODULE;COMMENT" "SOURCES;COMMON;DEPENDS" ${ARGN})
	list(LENGTH C_SOURCES source_count)
	if (NOT DARLING_SWIFT_WMO AND DARLING_SWIFT_JOBS GREATER 1 AND NOT source_count LESS DARLING_SWIFT_PARALLEL_MIN_SOURCES)
		set(object_dir "${CMAKE_CURRENT_BINARY_DIR}/swift-objects/${name}")
		set(map "${CMAKE_CURRENT_BINARY_DIR}/swift-${name}-output-file-map.json")
		set(objects)
		set(entries)
		set(index 0)
		foreach(src IN LISTS C_SOURCES)
			set(object "${object_dir}/${index}.o")
			list(APPEND objects "${object}")
			list(APPEND entries "\"${src}\": {\"object\": \"${object}\"}")
			math(EXPR index "${index} + 1")
		endforeach()
		list(JOIN entries ",\n  " entries)
		file(MAKE_DIRECTORY "${object_dir}")
		file(WRITE "${map}" "{\n  ${entries}\n}\n")
		add_custom_command(
			OUTPUT ${objects} "${C_MODULE}"
			COMMAND "${SWIFT_COMPILER}" -c -no-whole-module-optimization -enable-batch-mode
				-j "${DARLING_SWIFT_JOBS}" -output-file-map "${map}"
				${C_SOURCES} ${C_COMMON} -emit-module -emit-module-path "${C_MODULE}"
			DEPENDS ${C_SOURCES} ${C_DEPENDS} "${map}"
			COMMENT "${C_COMMENT} (${DARLING_SWIFT_JOBS} jobs)"
			${_DARLING_SWIFT_JOB_POOL}
			VERBATIM
		)
		set(${out_var} ${objects} PARENT_SCOPE)
	else()
		add_custom_command(
			OUTPUT "${C_OBJECT}" "${C_MODULE}"
			COMMAND "${SWIFT_COMPILER}" -frontend -c ${C_SOURCES} ${C_COMMON}
				-emit-module-path "${C_MODULE}" -o "${C_OBJECT}"
			DEPENDS ${C_SOURCES} ${C_DEPENDS}
			COMMENT "${C_COMMENT}"
			VERBATIM
		)
		set(${out_var} "${C_OBJECT}" PARENT_SCOPE)
	endif()
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
	file(MAKE_DIRECTORY "${DARLING_SWIFT_MODULE_CACHE}")
	set(sources)
	foreach(src IN LISTS SWIFTOBJ_SOURCES)
		get_filename_component(src "${src}" ABSOLUTE)
		list(APPEND sources "${src}")
	endforeach()
	_darling_swift_archs(swift_archs)
	foreach(arch IN LISTS swift_archs)
		set(module_dir "${DARLING_SWIFT_MODULE_DIR}/${arch}")
		file(MAKE_DIRECTORY "${module_dir}")
		set(module_deps)
		foreach(dep IN LISTS SWIFTOBJ_MODULES)
			list(APPEND module_deps "${module_dir}/${dep}.swiftmodule")
		endforeach()
		_darling_swift_compile("${module}.${arch}" objects_${arch}
			OBJECT "${CMAKE_CURRENT_BINARY_DIR}/${module}.${arch}.swift.o"
			MODULE "${module_dir}/${module}.swiftmodule" SOURCES ${sources} DEPENDS ${module_deps}
			COMMON
				-target "${arch}-apple-macosx${DARLING_SWIFT_DEPLOYMENT_TARGET}"
				-sdk "${DARLING_SWIFT_SDK}" -resource-dir "${resource_dir}"
				-I "${DARLING_SWIFT_STDLIB_MODULES}" -I "${module_dir}"
				-module-cache-path "${DARLING_SWIFT_MODULE_CACHE}"
				-swift-version 5 -module-name "${module}" -parse-as-library -O
				${DARLING_SWIFT_SDK_SUBFRAMEWORKS}
				${SWIFTOBJ_FLAGS}
			COMMENT "Compiling the Swift module ${module} for ${arch}")
		set_source_files_properties(${objects_${arch}} PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
	endforeach()
	_darling_swift_fat_objects("${module}" objects objects ${swift_archs})
	set_source_files_properties(${objects} PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
	set(${module}_OBJECT ${objects} PARENT_SCOPE)
endfunction()

function(add_darling_swift_library module)
	cmake_parse_arguments(SWIFTLIB "FRAMEWORK" "INSTALL_NAME" "SOURCES;FLAGS;MODULES;WEAK_MODULES;OBJECTS;LINK_FLAGS;NATIVE_SOURCES;NATIVE_FLAGS;LIBRARIES" ${ARGN})

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
	file(MAKE_DIRECTORY "${DARLING_SWIFT_MODULE_CACHE}")

	set(module_libs)
	foreach(dep IN LISTS SWIFTLIB_MODULES)
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
	_darling_swift_archs(swift_archs)
	foreach(arch IN LISTS swift_archs)
		set(module_dir "${DARLING_SWIFT_MODULE_DIR}/${arch}")
		file(MAKE_DIRECTORY "${module_dir}")
		set(module_deps)
		foreach(dep IN LISTS SWIFTLIB_MODULES SWIFTLIB_WEAK_MODULES)
			list(APPEND module_deps "${module_dir}/${dep}.swiftmodule")
		endforeach()
		_darling_swift_compile("${module}.${arch}" objects_${arch}
			OBJECT "${CMAKE_CURRENT_BINARY_DIR}/swift${module}.${arch}.swift.o"
			MODULE "${module_dir}/${module}.swiftmodule" SOURCES ${sources} DEPENDS ${module_deps}
			COMMON
				-target "${arch}-apple-macosx${DARLING_SWIFT_DEPLOYMENT_TARGET}"
				-sdk "${DARLING_SWIFT_SDK}" -resource-dir "${resource_dir}"
				-I "${DARLING_SWIFT_STDLIB_MODULES}" -I "${module_dir}"
				-module-cache-path "${DARLING_SWIFT_MODULE_CACHE}"
				-swift-version 5 -module-name "${module}" -module-link-name "swift${module}" -autolink-force-load
				-enable-library-evolution -parse-as-library -O
				-define-availability "SwiftStdlib 5.7:macOS 13.0"
				${DARLING_SWIFT_SDK_SUBFRAMEWORKS}
				${SWIFTLIB_FLAGS}
			COMMENT "Compiling the Swift overlay ${module} for ${arch}")
		set_source_files_properties(${objects_${arch}} PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
	endforeach()
	_darling_swift_fat_objects("swift${module}" objects objects ${swift_archs})
	set_source_files_properties(${objects} PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)

	if (SWIFTLIB_INSTALL_NAME)
		set(DYLIB_INSTALL_NAME "${SWIFTLIB_INSTALL_NAME}")
	elseif (SWIFTLIB_FRAMEWORK)
		set(DYLIB_INSTALL_NAME "/System/Library/Frameworks/${module}.framework/Versions/A/${module}")
	else()
		set(DYLIB_INSTALL_NAME "/usr/lib/swift/libswift${module}.dylib")
	endif()
	set(DARLING_LIB_ARCHS ${swift_archs})

	add_darling_library(swift${module} SHARED ${objects} ${SWIFTLIB_OBJECTS} ${SWIFTLIB_NATIVE_SOURCES})
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
	if (SWIFTLIB_FRAMEWORK)
		set(framework_dir "${CMAKE_CURRENT_BINARY_DIR}/${module}.framework/Versions/A")
		set_target_properties(swift${module} PROPERTIES
			OUTPUT_NAME "${module}"
			PREFIX ""
			SUFFIX ""
			LIBRARY_OUTPUT_DIRECTORY "${framework_dir}"
			RUNTIME_OUTPUT_DIRECTORY "${framework_dir}"
		)
		add_custom_command(TARGET swift${module} POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E create_symlink "${module}.framework/Versions/A/${module}"
				"${CMAKE_CURRENT_BINARY_DIR}/libswift${module}.dylib"
			VERBATIM)
	endif()
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
	foreach(dep IN LISTS SWIFTLIB_MODULES SWIFTLIB_WEAK_MODULES)
		if (TARGET swift${dep})
			add_dependencies(swift${module} swift${dep})
		endif()
	endforeach()
	# The weak dependencies are found by ld64's -weak-l through the -L directory above (the module's autolink entry
	# names the same library, which stays weak).
	foreach(dep IN LISTS SWIFTLIB_WEAK_MODULES)
		set_property(TARGET swift${module} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-weak-lswift${dep}")
	endforeach()
	if (SWIFTLIB_FRAMEWORK)
		set(framework_root "libexec/darling/System/Library/Frameworks/${module}.framework")
		install(TARGETS swift${module} DESTINATION "${framework_root}/Versions/A")
		InstallSymlink(A "${CMAKE_INSTALL_PREFIX}/${framework_root}/Versions/Current")
		InstallSymlink("Versions/Current/${module}" "${CMAKE_INSTALL_PREFIX}/${framework_root}/${module}")
	else()
		install(TARGETS swift${module} DESTINATION libexec/darling/usr/lib/swift)
	endif()
endfunction()
