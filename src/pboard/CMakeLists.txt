project(pboard)

add_definitions(-nostdinc)

add_darling_executable(pbcopy pbcopy.m)
target_link_libraries(pbcopy AppKit Foundation objc)

add_darling_executable(pbpaste pbpaste.m)
target_link_libraries(pbpaste AppKit Foundation objc)

install(TARGETS pbcopy pbpaste DESTINATION libexec/darling/usr/bin)
