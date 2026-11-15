project(runningboardd)

add_darling_executable(runningboardd runningboardd.c handshake.c acquire.c archive.m)
target_link_libraries(runningboardd Foundation)

install(TARGETS runningboardd DESTINATION libexec/darling/usr/libexec)
install(FILES com.apple.runningboard.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
