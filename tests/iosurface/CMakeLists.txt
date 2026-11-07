# Host-side checks for IOSurface's DMA-BUF export. Built for the host, like
# the mldr and shellspawn tests, and run with ctest from the build directory.

add_executable(iosurface_dmabuf_host_test dmabuf_host.c)
target_include_directories(iosurface_dmabuf_host_test PRIVATE
	${CMAKE_SOURCE_DIR}/src/frameworks/IOSurface/src
	${CMAKE_SOURCE_DIR}/src/frameworks/OpenGL)
target_compile_options(iosurface_dmabuf_host_test PRIVATE -std=gnu11)
add_test(NAME iosurface_dmabuf_abi COMMAND iosurface_dmabuf_host_test)

# The import check needs the host EGL and GL development files.
find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
	pkg_check_modules(IOSURFACE_HOST_EGL QUIET egl gl)
endif()
if (IOSURFACE_HOST_EGL_FOUND)
	target_compile_definitions(iosurface_dmabuf_host_test PRIVATE HAVE_EGL)
	target_include_directories(iosurface_dmabuf_host_test PRIVATE ${IOSURFACE_HOST_EGL_INCLUDE_DIRS})
	target_link_libraries(iosurface_dmabuf_host_test PRIVATE ${IOSURFACE_HOST_EGL_LIBRARIES})
	add_test(NAME iosurface_dmabuf_gl COMMAND iosurface_dmabuf_host_test gl)
	set_tests_properties(iosurface_dmabuf_gl PROPERTIES SKIP_RETURN_CODE 77)
endif()
