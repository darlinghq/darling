if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    cmake_policy(SET CMP0011 NEW)
endif(COMMAND cmake_policy)

include(use_ld64)
include(CMakeParseArguments)

FUNCTION(add_darling_library name)
	foreach(f IN LISTS ARGN)
		set(files ${files} ${f})
	endforeach(f)

	set(CMAKE_SKIP_RPATH TRUE)
	add_library(${name} ${files})

	# Link using Apple's ld64
	set_target_properties(${name} PROPERTIES
		SUFFIX ".dylib"
		NO_SONAME TRUE)

	set_property(TARGET ${name} APPEND_STRING PROPERTY
		LINK_FLAGS " ${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

	if (DYLIB_INSTALL_NAME)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-dylib_install_name,${DYLIB_INSTALL_NAME} ")
	endif (DYLIB_INSTALL_NAME)

	use_ld64(${name})
ENDFUNCTION(add_darling_library)

FUNCTION(make_fat)
	set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
		LINK_FLAGS " -arch i386 -arch x86_64")
	set_property(TARGET ${ARGV} APPEND_STRING PROPERTY
		COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ -arch i386 -arch x86_64")
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
FUNCTION(add_circular name)
	cmake_parse_arguments(CIRCULAR "FAT" "LINK_FLAGS" "SOURCES;OBJECTS;SIBLINGS;STRONG_SIBLINGS;DEPENDENCIES" ${ARGN})
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
	add_darling_library("${name}_firstpass" SHARED ${all_objects})# $<TARGET_OBJECTS:${name}_obj> ${CIRCULAR_OBJECTS})
	set_property(TARGET "${name}_firstpass" APPEND_STRING PROPERTY LINK_FLAGS " ${CIRCULAR_LINK_FLAGS} -Wl,-flat_namespace -Wl,-undefined,suppress")

	foreach(dep ${CIRCULAR_STRONG_SIBLINGS})
		target_link_libraries("${name}_firstpass" PRIVATE "${dep}_firstpass")
	endforeach(dep)
	
	if (CIRCULAR_FAT)
		make_fat("${name}_firstpass")
	endif (CIRCULAR_FAT)

	# Then build the final product while linking against firstpass libraries
	add_darling_library(${name} SHARED ${all_objects}) #$<TARGET_OBJECTS:${name}_obj> ${CIRCULAR_OBJECTS})
	set_property(TARGET "${name}" APPEND_STRING PROPERTY LINK_FLAGS " ${CIRCULAR_LINK_FLAGS}")

	foreach(dep ${CIRCULAR_SIBLINGS})
		target_link_libraries("${name}" PRIVATE "${dep}_firstpass")
	endforeach(dep)
	target_link_libraries("${name}" PRIVATE ${CIRCULAR_DEPENDENCIES})

	if (CIRCULAR_FAT)
		make_fat(${name})
	endif (CIRCULAR_FAT)
ENDFUNCTION(add_circular)
