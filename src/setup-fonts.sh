#! /bin/bash

# This runs as root,
# with the current directory set to ${CMAKE_INSTALL_PREFIX}/libexec/darling

root=/Volumes/SystemRoot

for file in $(find /etc/fonts/ -type f); do
    sed "s|/usr/|$root/usr/|" $file > .$file
done

for link in $(find /etc/fonts/ -type l); do
    [ -L .$link ] || ln -s $root$(realpath $link) .$link
done
