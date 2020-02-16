project(open)

add_darling_executable(open open.m)

target_link_libraries(open Foundation)

install(TARGETS open DESTINATION libexec/darling/usr/bin)
