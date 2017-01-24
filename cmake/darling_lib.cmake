if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    cmake_policy(SET CMP0011 NEW)
endif(COMMAND cmake_policy)

include(use_ld64)

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

