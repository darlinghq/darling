
FUNCTION(add_darling_bundle name loader)
	add_darling_library(${name} ${ARGN})
	set_property(TARGET ${name} APPEND_STRING PROPERTY
		LINK_FLAGS " -Wl,-bundle ")

	if (loader)
		set_property(TARGET ${name} APPEND_STRING PROPERTY
			LINK_FLAGS " -Wl,-bundle_loader,${loader} ")
	endif (loader)
ENDFUNCTION(add_darling_bundle)

