project(pboard)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

include(darling_exe)

add_darling_executable(pbcopy pbcopy.m)
target_link_libraries(pbcopy AppKit Foundation objc)

add_darling_executable(pbpaste pbpaste.m)
target_link_libraries(pbpaste AppKit Foundation objc)

install(TARGETS pbcopy pbpaste DESTINATION libexec/darling/usr/bin)
