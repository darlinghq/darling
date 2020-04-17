project(libelfstub)

set(elfstub_sources
	elfcalls.c
	threads.c
)

if(TARGET_x86_64 OR TARGET_ARM64)
	add_executable(elfloader_dummy64 ${elfstub_sources})
	target_link_libraries(elfloader_dummy64 -lpthread -lrt -ldl)
	set_target_properties(elfloader_dummy64 PROPERTIES COMPILE_FLAGS "-fPIC"
							LINK_FLAGS "-pie -fPIC")
endif(TARGET_x86_64 OR TARGET_ARM64)

if(TARGET_i386)
	add_executable(elfloader_dummy32 ${elfstub_sources})
	target_link_libraries(elfloader_dummy32 -lpthread -lrt -ldl)
	set_target_properties(elfloader_dummy32 PROPERTIES COMPILE_FLAGS "-fPIC -m32"
							LINK_FLAGS "-pie -fPIC -m32")
endif(TARGET_i386)

if(TARGET_x86_64 AND TARGET_i386)
	install(TARGETS elfloader_dummy32 elfloader_dummy64 DESTINATION libexec/darling/usr/libexec)
elseif(TARGET_x86_64 OR TARGET_ARM64)
	install(TARGETS elfloader_dummy64 DESTINATION libexec/darling/usr/libexec)
elseif(TARGET_i386)
	install(TARGETS elfloader_dummy32 DESTINATION libexec/darling/usr/libexec)
endif(TARGET_x86_64 AND TARGET_i386)
