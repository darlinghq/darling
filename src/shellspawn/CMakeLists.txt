project(shellspawn)

add_definitions(-nostdinc)

add_darling_executable(shellspawn shellspawn.c duct_signals.c)

install(TARGETS shellspawn DESTINATION libexec/darling/usr/libexec)
install(FILES org.darlinghq.shellspawn.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)

