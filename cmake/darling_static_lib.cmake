FUNCTION(add_darling_static_library name)
	foreach(f IN LISTS ARGN)
		set(files ${files} ${f})
	endforeach(f)

	set(CMAKE_AR "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/x86_64-apple-darwin11-ar")
	set(CMAKE_RANLIB "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/ranlib")
	add_library(${name} STATIC ${files})
	add_dependencies(${name} ranlib x86_64-apple-darwin11-ar)
ENDFUNCTION(add_darling_static_library)

