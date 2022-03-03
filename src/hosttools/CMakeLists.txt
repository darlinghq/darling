project(hosttools)

set(DARLING_COREDUMP_SANITIZE OFF)

add_executable(darling-coredump
	src/coredump/main.c
)

target_include_directories(darling-coredump PRIVATE
	# TEMPORARY
	# this set of includes should probably be moved out because we probably want to be able to use Mach-O headers
	# in non-Darling compilation environments more often. by non-Darling, i mean when compiling directly for Linux
	# as we do in mldr and now here.
	../startup/mldr/include
)

install(TARGETS darling-coredump DESTINATION bin)

if (DARLING_COREDUMP_SANITIZE)
	target_compile_options(darling-coredump PRIVATE
		-fsanitize=address,undefined
		-fsanitize-recover=all
		-fno-omit-frame-pointer
		-g
		-Og
	)
	target_link_options(darling-coredump PRIVATE
		-fsanitize=address,undefined
		-fsanitize-recover=all
	)
endif()
