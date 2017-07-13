include(CMakeParseArguments)
include(darling_lib)
include(InstallSymlink)

function(add_framework name)
	cmake_parse_arguments(FRAMEWORK "CURRENT_VERSION;FAT;PRIVATE" "VERSION;LINK_FLAGS" "SOURCES;DEPENDENCIES;CIRCULAR_DEPENDENCIES" ${ARGN})
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

	if (FRAMEWORK_CURRENT_VERSION)
		InstallSymlink(${FRAMEWORK_VERSION} "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/${dir_name}/${name}.framework/Versions/Current")
		InstallSymlink("Versions/Current/${name}" "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/Library/${dir_name}/${name}.framework/${name}")
	endif (FRAMEWORK_CURRENT_VERSION)
endfunction(add_framework)


