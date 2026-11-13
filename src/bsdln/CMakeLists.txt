project(bsdln)

add_executable(bsdln
	ln.c
)
# Bionic has strlcpy() in libc and no libbsd
if (NOT DARLING_ANDROID_HOST)
	target_link_libraries(bsdln bsd)
endif()
