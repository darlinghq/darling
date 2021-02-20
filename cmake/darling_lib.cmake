if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    cmake_policy(SET CMP0011 NEW)
endif(COMMAND cmake_policy)

include(use_ld64)
include(CMakeParseArguments)
include(dsym)

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

	if (BUILD_TARGET_64BIT AND NOT DARLING_LIB_32BIT_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS  " -arch ${APPLE_ARCH_64BIT}")
		set_property(TARGET ${name} APPEND_STRING PROPERTY LINK_FLAGS " -arch ${APPLE_ARCH_64BIT}")
	endif (BUILD_TARGET_64BIT AND NOT DARLING_LIB_32BIT_ONLY)
	if (BUILD_TARGET_32BIT AND NOT DARLING_LIB_64BIT_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -arch ${APPLE_ARCH_32BIT}")
		set_property(TARGET ${name} APPEND_STRING PROPERTY LINK_FLAGS " -arch ${APPLE_ARCH_32BIT}")
	endif (BUILD_TARGET_32BIT AND NOT DARLING_LIB_64BIT_ONLY)

	use_ld64(${name})

	if ((NOT NO_DSYM) AND (NOT ${name}_NO_DSYM))
		dsym(${name})
	endif ((NOT NO_DSYM) AND (NOT ${name}_NO_DSYM))
ENDFUNCTION(add_darling_library)

FUNCTION(make_fat)
	if (BUILD_TARGET_32BIT AND BUILD_TARGET_64BIT)
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ -arch ${APPLE_ARCH_32BIT} -arch ${APPLE_ARCH_64BIT}")
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			LINK_FLAGS " -arch ${APPLE_ARCH_32BIT} -arch ${APPLE_ARCH_64BIT}")
		foreach(tgt ${ARGV})
			add_dependencies(${tgt} lipo)
		endforeach(tgt)
	elseif (BUILD_TARGET_32BIT)
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ -arch ${APPLE_ARCH_32BIT}")
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			LINK_FLAGS " -arch ${APPLE_ARCH_32BIT}")
		foreach(tgt ${ARGV})
			add_dependencies(${tgt} lipo)
		endforeach(tgt)
	elseif (BUILD_TARGET_64BIT)
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ -arch ${APPLE_ARCH_64BIT}")
		set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
			LINK_FLAGS " -arch ${APPLE_ARCH_64BIT}")
		foreach(tgt ${ARGV})
			add_dependencies(${tgt} lipo)
		endforeach(tgt)
	endif (BUILD_TARGET_32BIT AND BUILD_TARGET_64BIT)
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

function(add_darling_object_library name)
	cmake_parse_arguments(OBJECT_LIB "32BIT_ONLY;64BIT_ONLY" "" "" ${ARGN})
	foreach(f IN LISTS OBJECT_LIB_UNPARSED_ARGUMENTS)
		set(files ${files} ${f})
	endforeach(f)

	add_library(${name} OBJECT ${files})
	add_dependencies(${name} lipo)
	set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/")

	if (BUILD_TARGET_32BIT AND NOT OBJECT_LIB_64BIT_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -arch ${APPLE_ARCH_32BIT}")
	endif (BUILD_TARGET_32BIT AND NOT OBJECT_LIB_64BIT_ONLY)
	if (BUILD_TARGET_64BIT AND NOT OBJECT_LIB_32BIT_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -arch ${APPLE_ARCH_64BIT}")
	endif (BUILD_TARGET_64BIT AND NOT OBJECT_LIB_32BIT_ONLY)
endfunction(add_darling_object_library)
