cmake_policy(SET CMP0069 NEW)
cmake_policy(SET CMP0063 NEW)

include(darling_lib)

function(add_darling_static_library name)
	cmake_parse_arguments(STATIC_LIB "FAT;x86_64_ONLY;i386_ONLY" "" "SOURCES" ${ARGN})

	set(CMAKE_AR "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/x86_64-apple-darwin11-ar")
	set(CMAKE_RANLIB "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/x86_64-apple-darwin11-ranlib")
	add_library(${name} STATIC ${STATIC_LIB_SOURCES})

	set_property(TARGET ${name} APPEND_STRING PROPERTY COMPILE_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/")
	set_property(TARGET ${name} APPEND_STRING PROPERTY LINK_FLAGS " -B ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/")
	add_dependencies(${name} lipo)

	if (TARGET_x86_64 AND NOT STATIC_LIB_i386_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			COMPILE_FLAGS " -arch x86_64")
	endif ()

	if (TARGET_i386 AND NOT STATIC_LIB_x86_64_ONLY)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			COMPILE_FLAGS " -arch i386")
	endif ()

	if (STATIC_LIB_FAT)
		make_fat(${name})
	endif (STATIC_LIB_FAT)

	add_dependencies(${name} ranlib x86_64-apple-darwin11-ar)
endfunction(add_darling_static_library)
