project(xcselect)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -fblocks")

set(DYLIB_INSTALL_NAME "/usr/lib/libxcselect.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(xcselect xcselect.c)
target_link_libraries(xcselect system)

add_darling_executable(xcrun xcrun.c)
target_link_libraries(xcrun system xcselect)

add_darling_executable(xcode-select xcode-select.c)
target_link_libraries(xcode-select system xcselect)

function(add_shim name)
	add_darling_executable("${name}_shim" xcrun-shim.c)

	set_target_properties("${name}_shim"
		PROPERTIES
			OUTPUT_NAME "${name}"
			COMPILE_FLAGS "-DTOOL_NAME=\\\"${name}\\\""
	)
	target_link_libraries("${name}_shim" system xcselect)
	install(TARGETS "${name}_shim" DESTINATION libexec/darling/usr/bin)
endfunction(add_shim)

add_shim(nm)

install(TARGETS xcselect DESTINATION libexec/darling/usr/lib)
install(TARGETS xcrun xcode-select DESTINATION libexec/darling/usr/bin)

