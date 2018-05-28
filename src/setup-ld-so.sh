#! /bin/bash

# This runs as root,
# with the current directory set to ${CMAKE_INSTALL_PREFIX}/libexec/darling

for file in /etc/ld.so.conf $(find -L /etc/ld.so.conf.d/ -type f); do
    # Copy lines from e.g. /etc/ld.so.conf into ./etc/ld.so.conf,
    # prepending "/Volumes/SystemRoot" to each line that starts with a slash
    awk '/^\// { print "/Volumes/SystemRoot" $0 }; /^[^\/]/' $file > .$file
done

# These are the defaults used by the ELF dynamic linker.
# They don't need to be explicitly listed in /etc/ld.so.conf.
cat > ./etc/ld.so.conf.d/defaults.conf <<END
/Volumes/SystemRoot/lib
/Volumes/SystemRoot/usr/lib
END

unshare --mount bash -c "mount --rbind / Volumes/SystemRoot && ldconfig -r . -X"
