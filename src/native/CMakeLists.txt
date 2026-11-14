project(native)

include(wrap_elf)
find_package(PkgConfig QUIET)

if (COMPONENT_gui)
	set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "gui")

	wrap_elf(FreeType libfreetype.so)
	wrap_elf(jpeg libjpeg.so)
	wrap_elf(png libpng.so)
	wrap_elf(tiff libtiff.so)
	wrap_elf(gif libgif.so)
	wrap_elf(EGL libEGL.so)
	wrap_elf(fontconfig libfontconfig.so)
	wrap_elf(X11 libX11.so)
	wrap_elf(Xext libXext.so)
	wrap_elf(XRandR libXrandr.so)
	wrap_elf(Xcursor libXcursor.so)
	wrap_elf(xkbfile libxkbfile.so)

	# Wayland stack: wayland-client, wayland-cursor and xkbcommon form an indivisible
	# unit used only by Cocotron's Wayland.backend. Only generate ELF wrappers if the
	# full stack is present on the build host to prevent wrapgen dlopen() failures.
	if (PKG_CONFIG_FOUND)
		pkg_check_modules(WAYLAND_CLIENT QUIET wayland-client>=1.20)
		pkg_check_modules(WAYLAND_CURSOR QUIET wayland-cursor)
		pkg_check_modules(XKBCOMMON QUIET xkbcommon)
	endif()

	if (WAYLAND_CLIENT_FOUND AND WAYLAND_CURSOR_FOUND AND XKBCOMMON_FOUND)
		wrap_elf(wayland-client libwayland-client.so)
		wrap_elf(wayland-cursor libwayland-cursor.so)
		wrap_elf(xkbcommon libxkbcommon.so)
	else()
		message(STATUS "native: skipping Wayland ELF wrappers (missing complete wayland-client>=1.20, wayland-cursor, xkbcommon stack)")
	endif()

	wrap_elf(cairo libcairo.so)
	wrap_elf(dbus libdbus-1.so)
	wrap_elf(GL libGL.so "System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries")
	wrap_elf(GLU libGLU.so "System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries")
endif()
