project(ditto)

add_darling_executable(ditto ditto.c)
target_include_directories(ditto PRIVATE
	${CMAKE_SOURCE_DIR}/src/external/libarchive/libarchive/libarchive
)
target_link_libraries(ditto archive)

install(TARGETS ditto DESTINATION libexec/darling/usr/bin)
