project(darling-hostcerts)

add_definitions(-nostdinc)

add_darling_executable(darling-hostcerts-import darling-hostcerts-import.c)
target_link_libraries(darling-hostcerts-import system CoreFoundation Security)

install(TARGETS darling-hostcerts-import DESTINATION libexec/darling/usr/libexec)
install(FILES org.darlinghq.hostcerts.plist DESTINATION libexec/darling/System/Library/LaunchDaemons)

# Pristine copies of the anchor files generated from the shipped certificates (see
# darling-openssl_certificates), for restoring them when the host bundle is gone.
# They get a directory of their own so that older installed files cannot mix in.
add_dependencies(darling-hostcerts-import build-security-certificates)
install(FILES
	"${CMAKE_BINARY_DIR}/certs/Certificates.bundle/Contents/Resources/certsIndex.data"
	"${CMAKE_BINARY_DIR}/certs/Certificates.bundle/Contents/Resources/certsTable.data"
	DESTINATION libexec/darling/usr/libexec/darling-hostcerts
)
