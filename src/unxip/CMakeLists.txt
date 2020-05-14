project(unxip)

add_definitions(-nostdinc)

add_darling_executable(xip_extract_cpio xip_extract_cpio.c)
target_link_libraries(xip_extract_cpio system lzma xar)

install(TARGETS xip_extract_cpio DESTINATION libexec/darling/usr/libexec)
install(PROGRAMS unxip DESTINATION libexec/darling/usr/bin)

