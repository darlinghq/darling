# SPDX-License-Identifier: GPL-3.0-or-later
project(libtexturecodec)

# The texture decoder that ImageIO decodes ASTC data with (see README.md). It is plain C with no
# dependency but libSystem, a static library linked into the frameworks that decode textures, so it
# has no components and is not installed. It exports nothing: DTC_API is empty for the library and
# for everything that links it, and the sources are compiled with hidden visibility, so no dtc_
# symbol becomes part of the public surface of the image it is linked into.
add_darling_static_library(darling_texturecodec FAT SOURCES
	src/astc_block.c
	src/astc_decode.c
	src/astc_endpoint.c
	src/astc_file.c
	src/astc_ise.c
	src/astc_partition.c
)
target_include_directories(darling_texturecodec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(darling_texturecodec PUBLIC DTC_API=)
target_compile_options(darling_texturecodec PRIVATE -fvisibility=hidden)
