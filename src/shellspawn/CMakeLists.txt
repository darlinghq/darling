project(shellspawn)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

include(darling_exe)

add_darling_executable(shellspawn shellspawn.c duct_signals.c)

install(TARGETS shellspawn DESTINATION libexec/darling/usr/libexec)
install(FILES org.darlinghq.shellspawn.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)

