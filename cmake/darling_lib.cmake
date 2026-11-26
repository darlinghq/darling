if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    cmake_policy(SET CMP0011 NEW)
endif(COMMAND cmake_policy)

include(use_ld64)
include(CMakeParseArguments)
include(dsym)
include(architecture)

# _darling_select_archs(<out-var> <32bit-only> <64bit-only> <archs>): the enabled slices, keeping
# only the 32-bit or 64-bit ones when asked to and only those in <archs> when it is not empty.
function(_darling_select_archs out only_32 only_64 archs)
	set(filter "")
	if (only_32)
		list(APPEND filter 32BIT_ONLY)
	endif ()
	if (only_64)
		list(APPEND filter 64BIT_ONLY)
	endif ()
	darling_archs(selected ${filter})
	if (archs)
		set(wanted ${selected})
		set(selected "")
		foreach (arch IN LISTS wanted)
			if (arch IN_LIST archs)
				list(APPEND selected ${arch})
			endif ()
		endforeach ()
	endif ()
	set(${out} "${selected}" PARENT_SCOPE)
endfunction()

# _darling_lib_archs(<out-var>): the slices add_darling_library() builds, from the variables a
# caller may set around it: DARLING_LIB_ARCHS (a list of slices), DARLING_LIB_32BIT_ONLY and
# DARLING_LIB_64BIT_ONLY. Without them, every enabled slice.
function(_darling_lib_archs out)
	_darling_select_archs(selected "${DARLING_LIB_32BIT_ONLY}" "${DARLING_LIB_64BIT_ONLY}" "${DARLING_LIB_ARCHS}")
	set(${out} "${selected}" PARENT_SCOPE)
endfunction()

FUNCTION(add_darling_library name)
	foreach(f IN LISTS ARGN)
		set(files ${files} ${f})
	endforeach(f)

	set(CMAKE_SKIP_RPATH TRUE)
	add_library(${name} SHARED ${files})

	# Link using Apple's ld64
	set_target_properties(${name} PROPERTIES
		SUFFIX ".dylib"
		NO_SONAME TRUE)

	set_target_properties(${name} PROPERTIES PREFIX "lib")

	set_property(TARGET ${name} APPEND_STRING PROPERTY
		LINK_FLAGS " ${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

	if (DYLIB_INSTALL_NAME)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-dylib_install_name,${DYLIB_INSTALL_NAME} ")
	endif (DYLIB_INSTALL_NAME)
	if (DYLIB_COMPAT_VERSION)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-compatibility_version,${DYLIB_COMPAT_VERSION} ")
	else (DYLIB_COMPAT_VERSION)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-compatibility_version,1.0.0 ")
	endif (DYLIB_COMPAT_VERSION)
	if (DYLIB_CURRENT_VERSION)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-current_version,${DYLIB_CURRENT_VERSION} ")
	endif (DYLIB_CURRENT_VERSION)

	set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS  " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/")
	add_dependencies(${name} lipo)

	_darling_lib_archs(lib_archs)
	if (NOT lib_archs)
		message(FATAL_ERROR "add_darling_library(${name}): none of the requested slices is enabled (DARLING_ARCHS: ${DARLING_ARCHS})")
	endif ()
	darling_arch_flags(lib_arch_flags ${lib_archs})
	set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " ${lib_arch_flags}")
	set_property(TARGET ${name} APPEND_STRING PROPERTY LINK_FLAGS " ${lib_arch_flags}")

	use_ld64(${name})

	if ((NOT NO_DSYM) AND (NOT ${name}_NO_DSYM))
		dsym(${name})
	endif ((NOT NO_DSYM) AND (NOT ${name}_NO_DSYM))
ENDFUNCTION(add_darling_library)

# make_fat(<target>...): builds the targets for every enabled slice.
FUNCTION(make_fat)
	darling_arch_flags(fat_arch_flags ${DARLING_ARCHS})
	set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
		COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ ${fat_arch_flags}")
	set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
		LINK_FLAGS " ${fat_arch_flags}")
	foreach(tgt ${ARGV})
		add_dependencies(${tgt} lipo)
	endforeach(tgt)
ENDFUNCTION(make_fat)

# add_circular(name ...)
#
# Creates a shared library with circular/mutual dependencies on other libraries.
#
# Typical use: circular dependencies between libs in /usr/lib/system
#
# Parameters:
# * SOURCES: Sources to build the library from. This function will internally create an object library for them.
# * OBJECTS: Object libraries to build the library from. (Because we cannot add object libraries into object libraries.)
# * SIBLINGS: Which "circular" libraries we should link against in the final pass.
#             All specified libs must also be built with this function!
# * STRONG_SIBLINGS: Which "circular" libraries we should link against in the 1st pass.
#             Only use for special cases, e.g. if this library reexports from this strong siblibg
#             and another sibling depends on this reexport's existence.
# * DEPENDENCIES: Which standard libraries we should link against.
# * LINK_FLAGS: Extra linker flags, e.g. an alias list
# * UPWARD: Add an upward dependency. This affects initialization order. The specified dependency will only be
#             loaded and initialized after the current library is fully loaded. This is needed to break
#             certain dependency issues, esp. if a libSystem sublibrary depends on a library that depends on libSystem
#             and has initializers. dyld bails out unless this dependency is upward.
# * STRONG_DEPENDENCIES: Which regular dependencies we should link against in the firstpass.
#             Just like STRONG_SIBLINGS, this should only be used in special cases. The intended use case is when
#             another sibling depends on symbols linked into this sibling from dependencies (usually static libraries).
#             Current use case is for Security to link to its static libraries in the firstpass,
#             because coretls_cfhelpers depends on them.
FUNCTION(add_circular name)
	cmake_parse_arguments(CIRCULAR "FAT" "LINK_FLAGS" "SOURCES;OBJECTS;SIBLINGS;STRONG_SIBLINGS;DEPENDENCIES;UPWARD;STRONG_DEPENDENCIES" ${ARGN})
	#message(STATUS "${name} sources: ${CIRCULAR_SOURCES}")
	#message(STATUS "${name} siblings ${CIRCULAR_SIBLINGS}")
	#message(STATUS "${name} deps: ${CIRCULAR_DEPENDENCIES}")

	set(all_objects "${CIRCULAR_OBJECTS}")

	# First create an object library for all sources with the aim to avoid rebuild
	if (CIRCULAR_SOURCES)
		add_library("${name}_obj" OBJECT ${CIRCULAR_SOURCES})
		if (CIRCULAR_FAT)
			make_fat("${name}_obj")
		endif (CIRCULAR_FAT)
		set(all_objects "${all_objects};$<TARGET_OBJECTS:${name}_obj>")
	endif (CIRCULAR_SOURCES)

	# Then create a shared Darling library, while ignoring all dependencies and using flat namespace
	add_darling_library("${name}_firstpass" SHARED ${all_objects})
	set_property(TARGET "${name}_firstpass" APPEND_STRING PROPERTY LINK_FLAGS " ${CIRCULAR_LINK_FLAGS} -Wl,-flat_namespace -Wl,-undefined,suppress")

	foreach(dep ${CIRCULAR_STRONG_SIBLINGS})
		target_link_libraries("${name}_firstpass" PRIVATE "${dep}_firstpass")
	endforeach(dep)

	# strong dependencies are linked in the firstpass
	target_link_libraries("${name}_firstpass" PRIVATE ${CIRCULAR_STRONG_DEPENDENCIES})
	
	if (CIRCULAR_FAT)
		make_fat("${name}_firstpass")
	endif (CIRCULAR_FAT)

	# Then build the final product while linking against firstpass libraries
	add_darling_library(${name} SHARED ${all_objects})

	foreach(dep ${CIRCULAR_SIBLINGS})
		target_link_libraries("${name}" PRIVATE "${dep}_firstpass")
	endforeach(dep)
	foreach(dep ${CIRCULAR_UPWARD})
		#get_property(lib_location TARGET ${dep} PROPERTY LOCATION_${CMAKE_BUILD_TYPE})
		#set_property(TARGET "${name}" APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-upward_library,${lib_location}")
		target_link_libraries("${name}" PRIVATE -Wl,-upward_library,$<TARGET_FILE:${dep}_firstpass>)
		add_dependencies("${name}" "${dep}_firstpass")
	endforeach(dep)
	
	get_property(dylib_files GLOBAL PROPERTY FIRSTPASS_MAP)
	set_property(TARGET "${name}" APPEND_STRING PROPERTY LINK_FLAGS " ${CIRCULAR_LINK_FLAGS}")

	target_link_libraries("${name}" PRIVATE ${CIRCULAR_DEPENDENCIES})

	# strong dependencies are linked again in the finalpass
	target_link_libraries("${name}" PRIVATE ${CIRCULAR_STRONG_DEPENDENCIES})

	if (CIRCULAR_FAT)
		make_fat(${name})
	endif (CIRCULAR_FAT)
ENDFUNCTION(add_circular)

# add_darling_object_library(<name> [32BIT_ONLY | 64BIT_ONLY] <sources>...)
# add_darling_object_library(<name> [32BIT_ONLY | 64BIT_ONLY] [ARCHS <arch>...] SOURCES <sources>...)
#
# An object library compiled for every enabled slice, or only for the enabled 32-bit or 64-bit
# slices, or only for those of ARCHS that are enabled. The object files are fat when that is more
# than one slice. With ARCHS the sources follow SOURCES.
function(add_darling_object_library name)
	cmake_parse_arguments(OBJECT_LIB "32BIT_ONLY;64BIT_ONLY" "" "ARCHS;SOURCES" ${ARGN})
	foreach(f IN LISTS OBJECT_LIB_UNPARSED_ARGUMENTS OBJECT_LIB_SOURCES)
		set(files ${files} ${f})
	endforeach(f)

	if ("ARCHS" IN_LIST ARGN AND NOT OBJECT_LIB_ARCHS)
		message(FATAL_ERROR "add_darling_object_library(${name}): ARCHS is given without any slice")
	endif ()

	add_library(${name} OBJECT ${files})
	add_dependencies(${name} lipo)
	# Set through COMPILE_OPTIONS rather than COMPILE_FLAGS: a caller that sets COMPILE_FLAGS on the
	# library afterwards (for a -D, say) replaces that property, which would silently drop the
	# slices and leave an object file for the default slice only.
	target_compile_options(${name} PRIVATE "SHELL:-B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/")

	_darling_select_archs(object_archs "${OBJECT_LIB_32BIT_ONLY}" "${OBJECT_LIB_64BIT_ONLY}" "${OBJECT_LIB_ARCHS}")
	if (NOT object_archs)
		message(FATAL_ERROR "add_darling_object_library(${name}): none of the requested slices is enabled (DARLING_ARCHS: ${DARLING_ARCHS})")
	endif ()
	foreach (arch IN LISTS object_archs)
		target_compile_options(${name} PRIVATE "SHELL:-arch ${arch}")
	endforeach ()
endfunction(add_darling_object_library)
