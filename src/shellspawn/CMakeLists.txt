project(shellspawn)

add_definitions(-nostdinc)

add_darling_executable(shellspawn shellspawn.c duct_signals.c)
add_darling_executable(shellsession shellsession.c)

install(TARGETS shellspawn shellsession DESTINATION libexec/darling/usr/libexec)
install(FILES org.darlinghq.shellsession.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
install(FILES org.darlinghq.shellspawn.plist DESTINATION libexec/darling/System/Library/LaunchAgents)

