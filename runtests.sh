#!/bin/sh

set -e

for i in mach/*.bin; do
    echo "Running $i"
    ./ld-mac ./$i
done

MAC_TOOL_DIR=/usr/i686-apple-darwin10
MAC_BIN_DIR=$MAC_TOOL_DIR/usr/bin

echo "Running gcc -c mach/hello.c"
$MAC_BIN_DIR/gcc -c mach/hello.c
echo "Running gcc mach/hello.c"
$MAC_BIN_DIR/gcc mach/hello.c
echo "Running gcc -g mach/hello.c"
PATH=$MAC_BIN_DIR $MAC_BIN_DIR/gcc -g mach/hello.c

echo
echo '*** ALL TESTS PASS ***'
