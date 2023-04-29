#
# Takes a single component and returns all its dependencies in `COMPONENT_DEPS`
#
function(darling_parse_components_get_deps COMPONENT)
	if (("${COMPONENT}" STREQUAL "system") OR ("${COMPONENT}" STREQUAL "python") OR ("${COMPONENT}" STREQUAL "ruby") OR ("${COMPONENT}" STREQUAL "perl"))
		set(COMPONENT_DEPS core PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "cli")
		set(COMPONENT_DEPS system PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "cli_dev")
		set(COMPONENT_DEPS cli python ruby perl dev_gui_common dev_gui_frameworks_common dev_gui_stubs_common PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "cli_extra")
		set(COMPONENT_DEPS cli PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "gui")
		set(COMPONENT_DEPS system dev_gui_common iokitd PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "gui_frameworks")
		set(COMPONENT_DEPS gui dev_gui_frameworks_common PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "gui_stubs")
		set(COMPONENT_DEPS gui dev_gui_stubs_common PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "dev_gui_common")
		set(COMPONENT_DEPS system PARENT_SCOPE)
	elseif (("${COMPONENT}" STREQUAL "dev_gui_frameworks_common") OR ("${COMPONENT}" STREQUAL "dev_gui_stubs_common"))
		set(COMPONENT_DEPS dev_gui_common PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "jsc")
		set(COMPONENT_DEPS system PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "webkit")
		set(COMPONENT_DEPS dev_gui_frameworks_common dev_gui_stubs_common PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "iokitd")
		set(COMPONENT_DEPS system PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "stock")
		set(COMPONENT_DEPS cli python ruby perl dev_gui_common dev_gui_frameworks_common dev_gui_stubs_common gui_frameworks gui_stubs PARENT_SCOPE)
	elseif ("${COMPONENT}" STREQUAL "all")
		set(COMPONENT_DEPS stock jsc webkit cli_extra cli_dev_gui_stubs PARENT_SCOPE)
	endif()
endfunction()

function(darling_parse_components DARLING_RAW_COMPONENTS)
	string(REPLACE "," ";" RAW_COMPONENTS_LIST "${DARLING_RAW_COMPONENTS}")

	set(COMPONENTS_TO_PROCESS "")

	foreach(RAW_COMPONENT IN LISTS RAW_COMPONENTS_LIST)
		string(STRIP "${RAW_COMPONENT}" STRIPPED_COMPONENT)
		string(TOLOWER "${STRIPPED_COMPONENT}" COMPONENT)

		list(APPEND COMPONENTS_TO_PROCESS "${COMPONENT}")
	endforeach()

	list(REMOVE_DUPLICATES COMPONENTS_TO_PROCESS)

	set(COMPONENTS_LIST "")

	# iterate through the list and add each component along with its dependencies.
	# this also handles circular dependencies between components (in case we ever have such dependency situations arise).
	while (COMPONENTS_TO_PROCESS)
		# get the next component and remove it from the list to process
		list(GET COMPONENTS_TO_PROCESS 0 COMPONENT)
		list(REMOVE_AT COMPONENTS_TO_PROCESS 0)

		# add it to the list of processed components
		list(APPEND COMPONENTS_LIST "${COMPONENT}")
		list(REMOVE_DUPLICATES COMPONENTS_LIST)

		# get its dependencies (into `COMPONENT_DEPS`)
		darling_parse_components_get_deps("${COMPONENT}")

		# add them to the list of components to process
		list(APPEND COMPONENTS_TO_PROCESS ${COMPONENT_DEPS})
		list(REMOVE_DUPLICATES COMPONENTS_TO_PROCESS)

		# remove components that we've already processed
		list(REMOVE_ITEM COMPONENTS_TO_PROCESS ${COMPONENTS_LIST})
	endwhile()

	# the "core" component is always on
	set(COMPONENT_core ON PARENT_SCOPE)

	# first, set every component off
	foreach(COMPONENT IN ITEMS system python ruby perl cli cli_dev cli_extra gui gui_frameworks gui_stubs jsc webkit dev_gui_common dev_gui_frameworks_common dev_gui_stubs_common)
		set("COMPONENT_${COMPONENT}" OFF PARENT_SCOPE)
	endforeach()

	# now turn on each component that was requested
	foreach(COMPONENT IN LISTS COMPONENTS_LIST)
		message(NOTICE "Including component: ${COMPONENT}")
		set("COMPONENT_${COMPONENT}" ON PARENT_SCOPE)
	endforeach()
endfunction()
