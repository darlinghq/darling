project(libDiagnosticMessagesClient)

set(DYLIB_INSTALL_NAME "/usr/lib/libDiagnosticMessagesClient.dylib")
add_darling_library(DiagnosticMessagesClient SHARED
	src/functions.c
)
target_link_libraries(DiagnosticMessagesClient system)
make_fat(DiagnosticMessagesClient)

install(TARGETS DiagnosticMessagesClient DESTINATION libexec/darling/usr/lib)
