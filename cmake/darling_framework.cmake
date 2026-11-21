include(CMakeParseArguments)
include(darling_lib)
include(swift_dylib_map)
include(darling_open_source_sdk)
include(InstallSymlink)

define_property(TARGET PROPERTY DYLIB_INSTALL_NAME BRIEF_DOCS "Stores the DYLIB_INSTALL_NAME of the framework's main binary"
	FULL_DOCS "Used to make reexporting child frameworks less painful.")

function(add_framework name)
	cmake_parse_arguments(FRAMEWORK "CURRENT_VERSION;FAT;PRIVATE;IOSSUPPORT;IOSSUPPORT_ALIAS;CIRCULAR;NO_INSTALL" "VERSION;LINK_FLAGS;PARENT;PARENT_VERSION;TARGET_NAME;PARENT_COMPONENT;PARENT_DIR"
		"SOURCES;DEPENDENCIES;CIRCULAR_DEPENDENCIES;RESOURCES;UPWARD_DEPENDENCIES;OBJECTS;STRONG_DEPENDENCIES" ${ARGN})

	if (FRAMEWORK_NO_INSTALL)
		set(EXCLUDE_FROM_ALL_ARG "EXCLUDE_FROM_ALL")
	else()
		set(EXCLUDE_FROM_ALL_ARG "")
	endif()

	if (DEFINED FRAMEWORK_TARGET_NAME)
		set(my_name "${FRAMEWORK_TARGET_NAME}")
	elseif (FRAMEWORK_CURRENT_VERSION)
		set(my_name "${name}")
	else()
		set(my_name "${name}_${FRAMEWORK_VERSION}")
	endif()

	if (FRAMEWORK_PRIVATE)
		set(dir_name "PrivateFrameworks")
	else (FRAMEWORK_PRIVATE)
		set(dir_name "Frameworks")
	endif (FRAMEWORK_PRIVATE)

	if (FRAMEWORK_IOSSUPPORT)
		set(sys_library_dir "System/iOSSupport/System/Library")
	else (FRAMEWORK_IOSSUPPORT)
		set(sys_library_dir "System/Library")
	endif (FRAMEWORK_IOSSUPPORT)

	set(root_dir "/${sys_library_dir}/${dir_name}")

	if (FRAMEWORK_PARENT_DIR)
		set(root_dir "${FRAMEWORK_PARENT_DIR}")
	endif()

	if (NOT (root_dir MATCHES "^/"))
		set(root_dir "/${root_dir}")
	endif()

	if(DEFINED FRAMEWORK_PARENT)
		if(NOT DEFINED FRAMEWORK_PARENT_VERSION)
			# 99% of the time it's version A
			set(FRAMEWORK_PARENT_VERSION "A")
		endif(NOT DEFINED FRAMEWORK_PARENT_VERSION)
		if (DEFINED FRAMEWORK_PARENT_COMPONENT)
			set(COMPONENT_ARG COMPONENT "${FRAMEWORK_PARENT_COMPONENT}")
		else()
			set(COMPONENT_ARG "")
		endif()
		InstallSymlink(Versions/Current/Frameworks
			"${CMAKE_INSTALL_PREFIX}/libexec/darling${root_dir}/${FRAMEWORK_PARENT}.framework/Frameworks"
			${EXCLUDE_FROM_ALL_ARG} ${COMPONENT_ARG})
		set(root_dir "${root_dir}/${FRAMEWORK_PARENT}.framework/Versions/${FRAMEWORK_PARENT_VERSION}/Frameworks")
	endif(DEFINED FRAMEWORK_PARENT)

	set(DYLIB_INSTALL_NAME "${root_dir}/${name}.framework/Versions/${FRAMEWORK_VERSION}/${name}")

	if (FRAMEWORK_CIRCULAR OR FRAMEWORK_CIRCULAR_DEPENDENCIES OR FRAMEWORK_UPWARD_DEPENDENCIES)
		if (FRAMEWORK_FAT)
			set (FRAMEWORK_FAT_ARG FAT)
		else (FRAMEWORK_FAT)
			set (FRAMEWORK_FAT_ARG)
		endif (FRAMEWORK_FAT)
		add_circular(${my_name}
			${FRAMEWORK_FAT_ARG}
			SOURCES ${FRAMEWORK_SOURCES}
			SIBLINGS ${FRAMEWORK_CIRCULAR_DEPENDENCIES}
			UPWARD ${FRAMEWORK_UPWARD_DEPENDENCIES}
			OBJECTS ${FRAMEWORK_OBJECTS}
			STRONG_DEPENDENCIES ${FRAMEWORK_STRONG_DEPENDENCIES}
		)

	else (FRAMEWORK_CIRCULAR OR FRAMEWORK_CIRCULAR_DEPENDENCIES OR FRAMEWORK_UPWARD_DEPENDENCIES)
		add_darling_library(${my_name} SHARED ${FRAMEWORK_SOURCES} ${FRAMEWORK_OBJECTS})
		target_link_libraries(${my_name} PRIVATE ${FRAMEWORK_STRONG_DEPENDENCIES})

		if (FRAMEWORK_FAT)
			make_fat(${my_name})
		endif (FRAMEWORK_FAT)

	endif (FRAMEWORK_CIRCULAR OR FRAMEWORK_CIRCULAR_DEPENDENCIES OR FRAMEWORK_UPWARD_DEPENDENCIES)
	
	set_property(TARGET ${my_name} PROPERTY DYLIB_INSTALL_NAME ${DYLIB_INSTALL_NAME})

	if (FRAMEWORK_CURRENT_VERSION AND NOT DEFINED FRAMEWORK_TARGET_NAME)
		add_library("${name}_${FRAMEWORK_VERSION}" ALIAS "${name}")
	endif()

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

	add_swift_dylib_map(${my_name})

	install(TARGETS ${my_name} DESTINATION "libexec/darling${root_dir}/${name}.framework/Versions/${FRAMEWORK_VERSION}/" ${EXCLUDE_FROM_ALL_ARG})

	if (FRAMEWORK_RESOURCES)
		if (FRAMEWORK_CURRENT_VERSION)
			InstallSymlink("Versions/Current/Resources" "${CMAKE_INSTALL_PREFIX}/libexec/darling${root_dir}/${name}.framework/Resources" ${EXCLUDE_FROM_ALL_ARG})
		endif (FRAMEWORK_CURRENT_VERSION)
		while (FRAMEWORK_RESOURCES)
			list(GET FRAMEWORK_RESOURCES 0 res_install_path)
			list(GET FRAMEWORK_RESOURCES 1 res_source_path)
			get_filename_component(res_install_dir ${res_install_path} DIRECTORY)
			get_filename_component(res_install_name ${res_install_path} NAME)
			install(FILES ${res_source_path}
				DESTINATION libexec/darling${root_dir}/${name}.framework/Versions/${FRAMEWORK_VERSION}/Resources/${res_install_dir}
				RENAME ${res_install_name} ${EXCLUDE_FROM_ALL_ARG})
			list(REMOVE_AT FRAMEWORK_RESOURCES 0 1)
		endwhile (FRAMEWORK_RESOURCES)
	endif()

	if (FRAMEWORK_CURRENT_VERSION)
		InstallSymlink(${FRAMEWORK_VERSION} "${CMAKE_INSTALL_PREFIX}/libexec/darling${root_dir}/${name}.framework/Versions/Current" ${EXCLUDE_FROM_ALL_ARG})
		InstallSymlink("Versions/Current/${name}" "${CMAKE_INSTALL_PREFIX}/libexec/darling${root_dir}/${name}.framework/${name}" ${EXCLUDE_FROM_ALL_ARG})
	endif()

	# Catalyst binaries hardcode /System/iOSSupport/... in their load commands, so
	# a framework shared with AppKit apps must be reachable at both paths.
	# IOSSUPPORT above *moves* the install; this aliases it, leaving it in place.
	if (FRAMEWORK_IOSSUPPORT_ALIAS)
		if (FRAMEWORK_IOSSUPPORT)
			message(FATAL_ERROR "${name}: IOSSUPPORT_ALIAS aliases the default install root into /System/iOSSupport, so it is meaningless with IOSSUPPORT, which moves the install there instead")
		endif()
		set(alias_dir "${CMAKE_INSTALL_PREFIX}/libexec/darling/System/iOSSupport/System/Library/${dir_name}")
		file(RELATIVE_PATH alias_target
			"${alias_dir}"
			"${CMAKE_INSTALL_PREFIX}/libexec/darling${root_dir}/${name}.framework")
		InstallSymlink("${alias_target}" "${alias_dir}/${name}.framework" ${EXCLUDE_FROM_ALL_ARG})
	endif()
endfunction(add_framework)

function(add_separated_framework name)
	cmake_parse_arguments(FRAMEWORK "CURRENT_VERSION;PRIVATE;IOSSUPPORT" "VERSION;LINK_FLAGS" "SOURCES;ARCHS;DEPENDENCIES;CIRCULAR_DEPENDENCIES;UPWARD_DEPENDENCIES;OBJECTS;STRONG_DEPENDENCIES;RESOURCES" ${ARGN})
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

	if (FRAMEWORK_IOSSUPPORT)
		set(sys_library_dir "System/iOSSupport/System/Library")
	else (FRAMEWORK_IOSSUPPORT)
		set(sys_library_dir "System/Library")
	endif (FRAMEWORK_IOSSUPPORT)

	set(DYLIB_INSTALL_NAME "/${sys_library_dir}/${dir_name}/${name}.framework/Versions/${FRAMEWORK_VERSION}/${name}")

	# reduce unnecessary recompilation by creating a single object library
	# `add_darling_object_library` automatically adds arch flags based on the same variables we use, so this works fine
	add_darling_object_library(${my_name}_obj ${FRAMEWORK_SOURCES})

	# One library per slice, named ${my_name}_<arch>, joined with lipo into ${my_name} (and, with
	# circular dependencies, the firstpass libraries into ${my_name}_firstpass).
	set(slice_targets "")
	set(firstpass_targets "")
	foreach (arch IN LISTS DARLING_ARCHS)
		set(slice ${my_name}_${arch})
		set(DARLING_LIB_ARCHS ${arch})
		if (FRAMEWORK_CIRCULAR_DEPENDENCIES OR FRAMEWORK_UPWARD_DEPENDENCIES)
			add_circular(${slice}
				SIBLINGS ${FRAMEWORK_CIRCULAR_DEPENDENCIES}
				UPWARD ${FRAMEWORK_UPWARD_DEPENDENCIES}
				OBJECTS $<TARGET_OBJECTS:${my_name}_obj> ${FRAMEWORK_OBJECTS}
				STRONG_DEPENDENCIES ${FRAMEWORK_STRONG_DEPENDENCIES}
			)
		else (FRAMEWORK_CIRCULAR_DEPENDENCIES OR FRAMEWORK_UPWARD_DEPENDENCIES)
			add_darling_library(${slice} $<TARGET_OBJECTS:${my_name}_obj> ${FRAMEWORK_OBJECTS})
			target_link_libraries(${slice} PRIVATE ${FRAMEWORK_STRONG_DEPENDENCIES})
		endif (FRAMEWORK_CIRCULAR_DEPENDENCIES OR FRAMEWORK_UPWARD_DEPENDENCIES)
		unset(DARLING_LIB_ARCHS)
		set_target_properties(${slice} PROPERTIES
			OUTPUT_NAME "${name}_${arch}"
			SUFFIX ""
			PREFIX ""
			ADHOC_CODESIGN_IDENTIFIER "${name}")
		if (NOT FRAMEWORK_CURRENT_VERSION)
			add_library("${slice}" ALIAS "${name}_${arch}")
		endif (NOT FRAMEWORK_CURRENT_VERSION)

		if (FRAMEWORK_DEPENDENCIES)
			target_link_libraries(${slice} PRIVATE ${FRAMEWORK_DEPENDENCIES})
		endif (FRAMEWORK_DEPENDENCIES)

		if (FRAMEWORK_LINK_FLAGS)
			set_property(TARGET ${slice} APPEND_STRING PROPERTY LINK_FLAGS " ${FRAMEWORK_LINK_FLAGS}")
		endif (FRAMEWORK_LINK_FLAGS)
		add_swift_dylib_map(${slice})

		list(APPEND slice_targets ${slice})
		if (FRAMEWORK_CIRCULAR_DEPENDENCIES)
			list(APPEND firstpass_targets ${slice}_firstpass)
		endif (FRAMEWORK_CIRCULAR_DEPENDENCIES)
	endforeach ()

	if (FRAMEWORK_CIRCULAR_DEPENDENCIES)
		darling_lipo(${my_name}_firstpass IMPORTED SHARED
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${my_name}_firstpass
			SLICES ${firstpass_targets})
	endif (FRAMEWORK_CIRCULAR_DEPENDENCIES)
	darling_lipo(${my_name} IMPORTED SHARED
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${my_name}
		SLICES ${slice_targets})

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${my_name} DESTINATION "libexec/darling/${sys_library_dir}/${dir_name}/${name}.framework/Versions/${FRAMEWORK_VERSION}/")

	if (FRAMEWORK_RESOURCES)
		if (FRAMEWORK_CURRENT_VERSION)
			InstallSymlink("Versions/Current/Resources" "${CMAKE_INSTALL_PREFIX}/libexec/darling/${sys_library_dir}/${dir_name}/${name}.framework/Resources")
		endif (FRAMEWORK_CURRENT_VERSION)
		while (FRAMEWORK_RESOURCES)
			list(GET FRAMEWORK_RESOURCES 0 res_install_path)
			list(GET FRAMEWORK_RESOURCES 1 res_source_path)
			get_filename_component(res_install_dir ${res_install_path} DIRECTORY)
			get_filename_component(res_install_name ${res_install_path} NAME)
			install(FILES ${res_source_path}
				DESTINATION libexec/darling/${sys_library_dir}/${dir_name}/${name}.framework/Versions/${FRAMEWORK_VERSION}/Resources/${res_install_dir}
				RENAME ${res_install_name})
			list(REMOVE_AT FRAMEWORK_RESOURCES 0 1)
		endwhile (FRAMEWORK_RESOURCES)
	endif (FRAMEWORK_RESOURCES)

	if (FRAMEWORK_CURRENT_VERSION)
		InstallSymlink(${FRAMEWORK_VERSION} "${CMAKE_INSTALL_PREFIX}/libexec/darling/${sys_library_dir}/${dir_name}/${name}.framework/Versions/Current")
		InstallSymlink("Versions/Current/${name}" "${CMAKE_INSTALL_PREFIX}/libexec/darling/${sys_library_dir}/${dir_name}/${name}.framework/${name}")
	endif (FRAMEWORK_CURRENT_VERSION)
endfunction(add_separated_framework)
