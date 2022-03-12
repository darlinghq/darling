project(bsdln)

if(FLATPAK_BUILD)
	# Flatpak, by default, adds additional compiler/linker arugments to cmake. This,
	# unfortunately, causes issues with our built macOS linker. As a workaround,
	# we set Flatpak to not add any additional arguments. However, this means we are
	# responsible for referencing the non-standard directories.

	# For more details, refer to the following issue: 
	# https://github.com/darlinghq/darling/issues/1113

	link_directories("/app/lib")
endif(FLATPAK_BUILD)

add_executable(bsdln
	ln.c
)
target_link_libraries(bsdln bsd)
