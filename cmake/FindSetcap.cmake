include(FindPackageHandleStandardArgs)

find_program(SETCAP_EXECUTABLE
	NAMES setcap
	PATHS /bin /usr/bin /sbin /usr/sbin
)

find_package_handle_standard_args(Setcap DEFAULT_MSG SETCAP_EXECUTABLE)

mark_as_advanced(SETCAP_EXECUTABLE)

