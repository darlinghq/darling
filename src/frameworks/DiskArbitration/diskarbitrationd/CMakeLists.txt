project(diskarbitrationd)

add_darling_executable(diskarbitrationd
	main.c
	DAModel.c
	DAServer.c
	DAOperations.c
)
target_include_directories(diskarbitrationd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(diskarbitrationd system CoreFoundation)

install(TARGETS diskarbitrationd DESTINATION libexec/darling/usr/libexec)
install(FILES com.apple.diskarbitrationd.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)
