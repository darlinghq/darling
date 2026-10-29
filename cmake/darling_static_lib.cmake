cmake_policy(SET CMP0069 NEW)
cmake_policy(SET CMP0063 NEW)

include(darling_lib)

function(add_darling_static_library name)
	cmake_parse_arguments(STATIC_LIB "FAT;64BIT_ONLY;32BIT_ONLY" "" "SOURCES" ${ARGN})

	set(CMAKE_AR "${CCTOOLS_AR}")
	set(CMAKE_RANLIB "${CCTOOLS_RANLIB}")
	add_library(${name} STATIC ${STATIC_LIB_SOURCES})

	set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -B ${CCTOOLS_LIPO_DIR}/")
	set_property(TARGET ${name} APPEND_STRING PROPERTY LINK_FLAGS " -B ${CCTOOLS_LIPO_DIR}/")
	add_dependencies(${name} lipo)

	if (BUILD_TARGET_64BIT AND NOT STATIC_LIB_32BIT_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			COMPILE_FLAGS " -arch ${APPLE_ARCH_64BIT}")
	endif ()

	if (BUILD_TARGET_32BIT AND NOT STATIC_LIB_64BIT_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			COMPILE_FLAGS " -arch ${APPLE_ARCH_32BIT}")
	endif ()

	if (STATIC_LIB_FAT)
		make_fat(${name})
	endif (STATIC_LIB_FAT)

	add_dependencies(${name} ranlib ${APPLE_TARGET_TRIPLET_PRIMARY}-ar)
endfunction(add_darling_static_library)
