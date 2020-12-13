project(libelfstub)

set(elfstub_sources
	elfcalls.c
	threads.c
)

if (BUILD_TARGET_64BIT)
	add_executable(elfloader_dummy64 ${elfstub_sources})
	target_link_libraries(elfloader_dummy64 -lpthread -lrt -ldl)
	set_target_properties(elfloader_dummy64 PROPERTIES COMPILE_FLAGS "-fPIC"
							LINK_FLAGS "-pie -fPIC")
endif ()

if (BUILD_TARGET_32BIT)
	add_executable(elfloader_dummy32 ${elfstub_sources})
	target_link_libraries(elfloader_dummy32 -lpthread -lrt -ldl)
	set_target_properties(elfloader_dummy32 PROPERTIES COMPILE_FLAGS "-fPIC -m32"
							LINK_FLAGS "-pie -fPIC -m32")
endif ()

if (BUILD_TARGET_64BIT AND BUILD_TARGET_32BIT)
	install(TARGETS elfloader_dummy32 elfloader_dummy64 DESTINATION libexec/darling/usr/libexec)
elseif (BUILD_TARGET_64BIT)
	install(TARGETS elfloader_dummy64 DESTINATION libexec/darling/usr/libexec)
elseif (BUILD_TARGET_32BIT)
	install(TARGETS elfloader_dummy32 DESTINATION libexec/darling/usr/libexec)
endif ()