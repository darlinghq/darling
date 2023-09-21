project(MobileKeyBag)

add_darling_static_library(MobileKeyBag
	SOURCES
		src/MobileKeyBag.c
)

target_include_directories(MobileKeyBag PUBLIC
	include
)
