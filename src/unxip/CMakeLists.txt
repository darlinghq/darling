project(unxip)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")

include_directories(
	${CMAKE_SOURCE_DIR}/src/xar
	${CMAKE_SOURCE_DIR}/src/external/liblzma/src/liblzma/api
)


add_darling_executable(xip_extract_cpio xip_extract_cpio.c)
target_link_libraries(xip_extract_cpio system lzma xar)

install(TARGETS xip_extract_cpio DESTINATION libexec/darling/usr/libexec)
install(FILES unxip DESTINATION libexec/darling/usr/bin)

