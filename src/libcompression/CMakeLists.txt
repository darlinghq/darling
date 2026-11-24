project(compression)

set(DYLIB_INSTALL_NAME "/usr/lib/libcompression.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

# LZFSE comes from the reference implementation, built into the library as on macOS.
set(LZFSE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/lzfse/src)

add_darling_library(compression
	src/compression.c
	src/lz4.c
	${LZFSE_DIR}/lzfse_decode.c
	${LZFSE_DIR}/lzfse_decode_base.c
	${LZFSE_DIR}/lzfse_encode.c
	${LZFSE_DIR}/lzfse_encode_base.c
	${LZFSE_DIR}/lzfse_fse.c
	${LZFSE_DIR}/lzvn_decode_base.c
	${LZFSE_DIR}/lzvn_encode_base.c
)
target_include_directories(compression PRIVATE ${LZFSE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../external/liblzma/src/liblzma/api)
set_source_files_properties(
	${LZFSE_DIR}/lzfse_decode.c
	${LZFSE_DIR}/lzfse_decode_base.c
	${LZFSE_DIR}/lzfse_encode.c
	${LZFSE_DIR}/lzfse_encode_base.c
	${LZFSE_DIR}/lzfse_fse.c
	${LZFSE_DIR}/lzvn_decode_base.c
	${LZFSE_DIR}/lzvn_encode_base.c
	PROPERTIES COMPILE_FLAGS "-Wno-unknown-pragmas -Wno-unused-variable"
)
make_fat(compression)
target_link_libraries(compression system z lzma)

install(TARGETS compression DESTINATION libexec/darling/usr/lib)
