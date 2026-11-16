project(pboard)

add_definitions(-nostdinc)

add_darling_executable(pbcopy pbcopy.m)
target_link_libraries(pbcopy AppKit Foundation objc)

add_darling_executable(pbpaste pbpaste.m)
target_link_libraries(pbpaste AppKit Foundation objc)

install(TARGETS pbcopy pbpaste DESTINATION libexec/darling/usr/bin)

add_darling_executable(pboard pboard.c)
target_link_libraries(pboard CoreFoundation)

install(TARGETS pboard DESTINATION libexec/darling/usr/libexec)
install(FILES com.apple.pboard.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
