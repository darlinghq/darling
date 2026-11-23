# SPDX-License-Identifier: GPL-3.0-or-later
# Host tests of the ASTC decoder, run with ctest. They are built with the build machine's
# compiler from the same sources as darling_texturecodec, and need no Darling build.
project(texturecodec-tests C)

set(DTC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(dtc_host STATIC
	${DTC_DIR}/src/astc_block.c
	${DTC_DIR}/src/astc_decode.c
	${DTC_DIR}/src/astc_endpoint.c
	${DTC_DIR}/src/astc_file.c
	${DTC_DIR}/src/astc_ise.c
	${DTC_DIR}/src/astc_partition.c
)
target_include_directories(dtc_host PUBLIC ${DTC_DIR}/include PRIVATE ${DTC_DIR}/src)
target_compile_options(dtc_host PRIVATE -std=c99 -Wall -Wextra -Wpedantic -Wconversion -Wno-sign-conversion)

foreach(name astc_block astc_decode astc_endpoint astc_file astc_image astc_ise astc_partition fuzz vectors)
	add_executable(test_${name} ${CMAKE_CURRENT_SOURCE_DIR}/test_${name}.c)
	target_include_directories(test_${name} PRIVATE ${DTC_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
	target_compile_options(test_${name} PRIVATE -std=c99 -Wall -Wextra -Wpedantic -Wconversion -Wno-sign-conversion)
	target_link_libraries(test_${name} PRIVATE dtc_host)
	# test_vectors finds its files relative to the library directory
	add_test(NAME texturecodec_${name} COMMAND test_${name} WORKING_DIRECTORY ${DTC_DIR})
endforeach()
