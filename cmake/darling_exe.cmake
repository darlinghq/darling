if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0011 NEW)
endif(COMMAND cmake_policy)


FUNCTION(add_darling_executable exe)
	foreach(f IN LISTS ARGN)
		set(files ${files} ${f})
	endforeach(f)

	SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
	SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	add_executable(${exe} ${files})
	target_link_libraries(${exe} system_c system_kernel system dyld "-Wl,--whole-archive" dexe_startup "-Wl,--no-whole-archive" "-shared")
	set_target_properties(${exe} PROPERTIES COMPILE_FLAGS "-Denviron=__darwin_environ -Ddlopen=__darwin_dlopen -Ddlclose=__darwin_dlclose -Ddlsym=__darwin_dlsym -Ddladdr=__darwin_dladdr -Ddlerror=__darwin_dlerror")
ENDFUNCTION(add_darling_executable)

