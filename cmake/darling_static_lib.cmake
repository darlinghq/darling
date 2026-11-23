cmake_policy(SET CMP0069 NEW)
cmake_policy(SET CMP0063 NEW)

include(darling_lib)

# add_darling_static_library(<name> [FAT] [32BIT_ONLY | 64BIT_ONLY] [ARCHS <arch>...] SOURCES <sources>...)
#
# A static library compiled for every enabled slice, or only for the enabled 32-bit or 64-bit
# slices, or only for those of ARCHS that are enabled (FAT: every enabled slice).
function(add_darling_static_library name)
	cmake_parse_arguments(STATIC_LIB "FAT;64BIT_ONLY;32BIT_ONLY" "" "SOURCES;ARCHS" ${ARGN})
	if ("ARCHS" IN_LIST ARGN AND NOT STATIC_LIB_ARCHS)
		message(FATAL_ERROR "add_darling_static_library(${name}): ARCHS is given without any slice")
	endif ()

	set(CMAKE_AR "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/${APPLE_TARGET_TRIPLET_PRIMARY}-ar")
	set(CMAKE_RANLIB "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/${APPLE_TARGET_TRIPLET_PRIMARY}-ranlib")
	add_library(${name} STATIC ${STATIC_LIB_SOURCES})

	set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/")
	set_property(TARGET ${name} APPEND_STRING PROPERTY LINK_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/")
	add_dependencies(${name} lipo)

	_darling_select_archs(static_archs "${STATIC_LIB_32BIT_ONLY}" "${STATIC_LIB_64BIT_ONLY}" "${STATIC_LIB_ARCHS}")
	if (NOT static_archs)
		message(FATAL_ERROR "add_darling_static_library(${name}): none of the requested slices is enabled (DARLING_ARCHS: ${DARLING_ARCHS})")
	endif ()
	darling_arch_flags(static_arch_flags ${static_archs})
	set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " ${static_arch_flags}")

	if (STATIC_LIB_FAT)
		make_fat(${name})
	endif (STATIC_LIB_FAT)

	add_dependencies(${name} ranlib ${APPLE_TARGET_TRIPLET_PRIMARY}-ar)
endfunction(add_darling_static_library)

# darling_lipo_static_library_slices(<name> <arch>...)
#
# Joins the static libraries <name>_<arch>, one per slice (thin archives: a Mach-O archive holds the
# members of one architecture), into the fat static library <name>, an imported target located at
# ${CMAKE_CURRENT_BINARY_DIR}/lib<name>.a, to link other targets against.
function(darling_lipo_static_library_slices name)
	set(slices "")
	foreach (arch IN LISTS ARGN)
		list(APPEND slices ${name}_${arch})
	endforeach ()
	if (NOT slices)
		message(FATAL_ERROR "darling_lipo_static_library_slices(${name}): no slices")
	endif ()
	darling_lipo(${name} IMPORTED STATIC
		OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lib${name}.a"
		SLICES ${slices})
endfunction()
