#! /bin/bash

# This runs as root,
# with the current directory set to ${CMAKE_INSTALL_PREFIX}/libexec/darling

for file in /etc/ld.so.conf $(find /etc/ld.so.conf.d/ -type f); do
    # Copy lines from e.g. /etc/ld.so.conf into ./etc/ld.so.conf,
    # prepending "/system-root" to each line that starts with a slash
    awk '/^\// { print "/system-root" $0 }; /^[^/]/' $file > .$file
done

unshare --mount bash -c "mount --rbind / system-root && ldconfig -r . -X"
