project(distnoted)

add_darling_executable(distnoted distnoted.c)
target_link_libraries(distnoted CoreFoundation)

install(TARGETS distnoted DESTINATION libexec/darling/usr/sbin)
install(FILES com.apple.distnoted.xpc.agent.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
