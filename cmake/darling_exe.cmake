if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0011 NEW)
endif(COMMAND cmake_policy)


FUNCTION(add_darling_executable exe)
	foreach(f IN LISTS ARGN)
		set(files ${files} ${f})
	endforeach(f)

	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib${SUFFIX}/darling")
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
	SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	add_executable(${exe} ${files})
	target_link_libraries(${exe} system -lpthread csu dyld)
	set_target_properties(${exe} PROPERTIES COMPILE_FLAGS "-Denviron=__darwin_environ -Ddlopen=__darwin_dlopen -Ddlclose=__darwin_dlclose -Ddlsym=__darwin_dlsym -Ddladdr=__darwin_dladdr")
ENDFUNCTION(add_darling_executable)

