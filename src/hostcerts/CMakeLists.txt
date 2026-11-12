project(darling-hostcerts)

add_definitions(-nostdinc)

add_darling_executable(darling-hostcerts-import darling-hostcerts-import.c)
target_link_libraries(darling-hostcerts-import system CoreFoundation Security)

install(TARGETS darling-hostcerts-import DESTINATION libexec/darling/usr/libexec)
install(FILES org.darlinghq.hostcerts.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
