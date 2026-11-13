project(buildtools)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../external/cctools-port/cctools/include)

add_executable(getuuid getuuid.c)
add_executable(elfdep elfdep.c)
add_executable(iconset2icns iconset2icns.c)

if (ADHOC_CODESIGN)
	# signs the linked Mach-O images (see use_ld64)
	find_package(OpenSSL REQUIRED COMPONENTS Crypto)
	add_executable(adhoc-sign adhoc-sign.c)
	target_link_libraries(adhoc-sign PRIVATE OpenSSL::Crypto)
endif (ADHOC_CODESIGN)

#add_subdirectory(Rez)
