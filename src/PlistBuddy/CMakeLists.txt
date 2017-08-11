project(PlistBuddy)

add_darling_executable(PlistBuddy
	PlistBuddy.c
)

target_link_libraries(PlistBuddy CoreFoundation)

install(TARGETS PlistBuddy DESTINATION libexec/darling/usr/libexec)

install(FILES PlistBuddy.8 DESTINATION libexec/darling/usr/share/man/man8)
