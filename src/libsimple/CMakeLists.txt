project(libsimple)

add_library(libsimple_darlingserver
	src/lock.c
)

target_include_directories(libsimple_darlingserver PUBLIC
	include
)

target_compile_definitions(libsimple_darlingserver PRIVATE
	LIBSIMPLE_LINUX=1
)
