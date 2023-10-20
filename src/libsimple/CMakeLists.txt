project(libsimple)

set(libsimple_sources
	src/lock.c
)

if (NOT DEFINED libsimple_linux_added)
	set(libsimple_linux_added TRUE PARENT_SCOPE)

	add_library(libsimple_darlingserver STATIC "${libsimple_sources}")

	target_include_directories(libsimple_darlingserver PUBLIC
		include
	)

	target_compile_definitions(libsimple_darlingserver PRIVATE
		LIBSIMPLE_LINUX=1
	)
else()
	add_darling_static_library(libsimple_darling
		FAT
		SOURCES "${libsimple_sources}"
	)

	target_include_directories(libsimple_darling PUBLIC
		include
	)

	target_compile_definitions(libsimple_darling PRIVATE
		LIBSIMPLE_DARLING=1
	)
endif()
