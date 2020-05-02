project(AppleSauce)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "0.0.0")

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src/external/libcxx/include)

add_compile_options(-Wno-error=stdlibcxx-not-found)

add_framework(AppleSauce
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		src/AppleSauce.cpp

	DEPENDENCIES
		system
		cxx
)
