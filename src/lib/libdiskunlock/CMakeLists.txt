project(libdiskunlock)

set(DYLIB_INSTALL_NAME "/usr/lib/libDiskUnlock.dylib")

# Boot Camp Assistant carries an LC_LOAD_DYLIB for this library and resolves
# DUUnlockVolume out of it, so it has to exist and be loadable: dyld fails a process
# outright when a load command names a file that is not there.
#
# The import table names the library "libDiskUnlock" here, without the .dylib
# suffix the load command uses, so a demand scan keyed on the load-command spelling
# misses it. Worth knowing before concluding a library is imported by nobody.
add_darling_library(DiskUnlock SHARED
	src/DiskUnlock.c
)
make_fat(DiskUnlock)
target_link_libraries(DiskUnlock PRIVATE system)

install(TARGETS DiskUnlock DESTINATION libexec/darling/usr/lib)
