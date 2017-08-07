include(darling_lib)

function(add_darling_static_library name)
	cmake_parse_arguments(STATIC_LIB "FAT" "" "SOURCES" ${ARGN})

	set(CMAKE_AR "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/x86_64-apple-darwin11-ar")
	set(CMAKE_RANLIB "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ranlib")
	add_library(${name} STATIC ${STATIC_LIB_SOURCES})

	if (STATIC_LIB_FAT)
		make_fat(${name})
	endif (STATIC_LIB_FAT)

	add_dependencies(${name} ranlib x86_64-apple-darwin11-ar)
endfunction(add_darling_static_library)
