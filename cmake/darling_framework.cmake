include(CMakeParseArguments)
include(darling_lib)
include(InstallSymlink)

function(add_framework name)
	cmake_parse_arguments(FRAMEWORK "CURRENT_VERSION;FAT;PRIVATE" "VERSION;LINK_FLAGS" "SOURCES;DEPENDENCIES;CIRCULAR_DEPENDENCIES;RESOURCES" ${ARGN})
	if (FRAMEWORK_CURRENT_VERSION)
		set(my_name "${name}")
	else (FRAMEWORK_CURRENT_VERSION)
		set(my_name "${name}_${FRAMEWORK_VERSION}")
	endif (FRAMEWORK_CURRENT_VERSION)

	if (FRAMEWORK_PRIVATE)
		set(dir_name "PrivateFrameworks")
	else (FRAMEWORK_PRIVATE)
		set(dir_name "Frameworks")
	endif (FRAMEWORK_PRIVATE)

	set(DYLIB_INSTALL_NAME "/System/Library/${dir_name}/${name}.framework/Versions/${FRAMEWORK_VERSION}/${name}")


	if (FRAMEWORK_CIRCULAR_DEPENDENCIES)
		if (FRAMEWORK_FAT)
			set (FRAMEWORK_FAT_ARG FAT)
		else (FRAMEWORK_FAT)
			set (FRAMEWORK_FAT_ARG)
		endif (FRAMEWORK_FAT)
		add_circular(${my_name}
			${FRAMEWORK_FAT_ARG}
			SOURCES ${FRAMEWORK_SOURCES}
			SIBLINGS ${FRAMEWORK_CIRCULAR_DEPENDENCIES})

	else (FRAMEWORK_CIRCULAR_DEPENDENCIES)
		add_darling_library(${my_name} SHARED ${FRAMEWORK_SOURCES})

		if (FRAMEWORK_FAT)
			make_fat(${my_name})
		endif (FRAMEWORK_FAT)

	endif (FRAMEWORK_CIRCULAR_DEPENDENCIES)


	if (FRAMEWORK_CURRENT_VERSION)
		add_library("${name}_${FRAMEWORK_VERSION}" ALIAS "${name}")
	endif (FRAMEWORK_CURRENT_VERSION)

	set_target_properties(${my_name} PROPERTIES
				OUTPUT_NAME "${name}"
		        SUFFIX ""
				PREFIX "")

	if (FRAMEWORK_DEPENDENCIES)
		target_link_libraries(${my_name} PRIVATE ${FRAMEWORK_DEPENDENCIES})
	endif (FRAMEWORK_DEPENDENCIES)

	if (FRAMEWORK_LINK_FLAGS)
		set_property(TARGET ${my_name} APPEND_STRING PROPERTY LINK_FLAGS " ${FRAMEWORK_LINK_FLAGS}")
	endif (FRAMEWORK_LINK_FLAGS)

	install(TARGETS ${my_name} DESTINATION "libexec/darling/System/Library/${dir_name}/${name}.framework/Versions/${FRAMEWORK_VERSION}/")

	if (FRAMEWORK_RESOURCES)
		if (FRAMEWORK_CURRENT_VERSION)
			InstallSymlink("Versions/Current/Resources" "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/${dir_name}/${name}.framework/Resources")
		endif (FRAMEWORK_CURRENT_VERSION)
		while (FRAMEWORK_RESOURCES)
			list(GET FRAMEWORK_RESOURCES 0 res_install_path)
			list(GET FRAMEWORK_RESOURCES 1 res_source_path)
			get_filename_component(res_install_dir ${res_install_path} DIRECTORY)
			get_filename_component(res_install_name ${res_install_path} NAME)
			install(FILES ${res_source_path}
				DESTINATION libexec/darling/System/Library/${dir_name}/${name}.framework/Versions/${FRAMEWORK_VERSION}/Resources/${res_install_dir}
				RENAME ${res_install_name})
			list(REMOVE_AT FRAMEWORK_RESOURCES 0 1)
		endwhile (FRAMEWORK_RESOURCES)
	endif (FRAMEWORK_RESOURCES)

	if (FRAMEWORK_CURRENT_VERSION)
		InstallSymlink(${FRAMEWORK_VERSION} "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/${dir_name}/${name}.framework/Versions/Current")
		InstallSymlink("Versions/Current/${name}" "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/${dir_name}/${name}.framework/${name}")
	endif (FRAMEWORK_CURRENT_VERSION)
endfunction(add_framework)

function(add_separated_framework name)
	cmake_parse_arguments(FRAMEWORK "CURRENT_VERSION;PRIVATE" "VERSION;LINK_FLAGS" "SOURCES;ARCHS;DEPENDENCIES;CIRCULAR_DEPENDENCIES" ${ARGN})
	if (FRAMEWORK_CURRENT_VERSION)
		set(my_name "${name}")
	else (FRAMEWORK_CURRENT_VERSION)
		set(my_name "${name}_${FRAMEWORK_VERSION}")
	endif (FRAMEWORK_CURRENT_VERSION)

	if (FRAMEWORK_PRIVATE)
		set(dir_name "PrivateFrameworks")
	else (FRAMEWORK_PRIVATE)
		set(dir_name "Frameworks")
	endif (FRAMEWORK_PRIVATE)

	set(DYLIB_INSTALL_NAME "/System/Library/${dir_name}/${name}.framework/Versions/${FRAMEWORK_VERSION}/${name}")


	add_darling_library(${my_name}_i386 SHARED ${FRAMEWORK_SOURCES})
	add_darling_library(${my_name}_x86_64 SHARED ${FRAMEWORK_SOURCES})

	set_target_properties(${my_name}_i386 PROPERTIES
				OUTPUT_NAME "${name}_i386"
						SUFFIX ""
				PREFIX "")
  set_target_properties(${my_name}_x86_64 PROPERTIES
				OUTPUT_NAME "${name}_x86_64"
						SUFFIX ""
				PREFIX "")
	set_property(TARGET ${my_name}_i386 APPEND_STRING PROPERTY
		COMPILE_FLAGS " -arch i386")
	set_property(TARGET ${my_name}_x86_64 APPEND_STRING PROPERTY
		COMPILE_FLAGS " -arch x86_64")
 set_property(TARGET ${my_name}_i386 APPEND_STRING PROPERTY
		LINK_FLAGS " -arch i386")
	set_property(TARGET ${my_name}_x86_64 APPEND_STRING PROPERTY
		LINK_FLAGS " -arch x86_64")

	if (NOT FRAMEWORK_CURRENT_VERSION)
		add_library("${my_name}_i386" ALIAS "${name}_i386")
		add_library("${my_name}_x86_64" ALIAS "${name}_x86_64")
	endif (NOT FRAMEWORK_CURRENT_VERSION)

	if (FRAMEWORK_DEPENDENCIES)
	  target_link_libraries(${my_name}_i386 PRIVATE ${FRAMEWORK_DEPENDENCIES})
	  target_link_libraries(${my_name}_x86_64 PRIVATE ${FRAMEWORK_DEPENDENCIES})
	endif (FRAMEWORK_DEPENDENCIES)

	if (FRAMEWORK_LINK_FLAGS)
		set_property(TARGET ${my_name}_i386 APPEND_STRING PROPERTY LINK_FLAGS " ${FRAMEWORK_LINK_FLAGS}")
		set_property(TARGET ${my_name}_x86_64 APPEND_STRING PROPERTY LINK_FLAGS " ${FRAMEWORK_LINK_FLAGS}")
	endif (FRAMEWORK_LINK_FLAGS)

	add_dependencies(${my_name}_x86_64 ${my_name}_i386)
	add_custom_command(TARGET ${my_name}_x86_64 POST_BUILD
		COMMAND ${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/misc/lipo
			-arch i386 $<TARGET_FILE:${my_name}_i386>
			-arch x86_64 $<TARGET_FILE:${my_name}_x86_64>
			-create
			-output
			${CMAKE_CURRENT_BINARY_DIR}/${my_name}
		COMMENT "Running lipo to create ${my_name}"
		BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${my_name}
	)
	add_library(${my_name} SHARED IMPORTED GLOBAL)
	set_target_properties(${my_name} PROPERTIES
		SUFFIX ""
		PREFIX ""
		IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${my_name}
	)
	add_dependencies(${my_name} ${my_name}_x86_64)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${my_name} DESTINATION "libexec/darling/System/Library/${dir_name}/${name}.framework/Versions/${FRAMEWORK_VERSION}/")

	if (FRAMEWORK_CURRENT_VERSION)
		InstallSymlink(${FRAMEWORK_VERSION} "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/${dir_name}/${name}.framework/Versions/Current")
		InstallSymlink("Versions/Current/${name}" "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/${dir_name}/${name}.framework/${name}")
	endif (FRAMEWORK_CURRENT_VERSION)
endfunction(add_separated_framework)
