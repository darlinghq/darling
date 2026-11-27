project(libdarlingdbus)

# The shared loader of Darling's dbus wrapper library (see src/native). It is a
# static library linked into each component that talks to the host's libdbus-1,
# so that there is one function table and one way of finding the library.
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_DBUS REQUIRED dbus-1)
include_directories(${PC_DBUS_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_darling_static_library(darling_dbus_loader FAT SOURCES darling_dbus.c)
# Its functions are for the component that links it, not part of that component's interface.
target_compile_options(darling_dbus_loader PRIVATE -fvisibility=hidden)
# the header needs the host's dbus headers as well
target_include_directories(darling_dbus_loader PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${PC_DBUS_INCLUDE_DIRS}
)
