#!/bin/sh
#
# Copyright 2011 Shinichiro Hamaji. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of  conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials
#      provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY Shinichiro Hamaji ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Shinichiro Hamaji OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

set -e

# Run small unit tests

for i in mach/*.bin; do
    echo "Running $i"
    ./ld-mac ./$i
done

# Run regressions tests with real compilers

MAC_TOOL_DIR=/usr/i686-apple-darwin10
MAC_BIN_DIR=$MAC_TOOL_DIR/usr/bin

apple() {
    gcc="$1"
    shift
    PATH=$MAC_BIN_DIR ./ld-mac $MAC_BIN_DIR/$gcc --sysroot=$MAC_TOOL_DIR "$@"
}

# Run GCC with ld-mac

echo "Running gcc -c mach/hello.c"
./ld-mac $MAC_BIN_DIR/gcc -c mach/hello.c
echo "Running gcc mach/hello.c"
./ld-mac $MAC_BIN_DIR/gcc mach/hello.c
echo "Running gcc -g mach/hello.c"
PATH=$MAC_BIN_DIR ./ld-mac $MAC_BIN_DIR/gcc -g mach/hello.c

# Run clang with ld-mac

echo "Running clang -c mach/hello.c"
./ld-mac $MAC_BIN_DIR/clang -c mach/hello.c

# Check dylib

echo "Running dylib tests"

apple gcc -g -dynamiclib mach/dylib/lib.c -o mach/dylib/lib.dylib
apple gcc -g mach/dylib/main.c mach/dylib/lib.dylib -o mach/dylib/main

echo "Running mach/dylib/main"
./ld-mac mach/dylib/main

echo "Running dylib tests"

apple gcc -g -dynamiclib mach/dylib/lib.c -o mach/dylib/lib.dylib
apple gcc -g mach/dylib/main.c mach/dylib/lib.dylib -o mach/dylib/main

echo "Running mach/dylib/main"
./ld-mac mach/dylib/main

apple gcc -g mach/dylib/dlfcn.c -o mach/dylib/dlfcn

echo "Running mach/dylib/dlfcn"
./ld-mac mach/dylib/dlfcn

# Check dylib with weak symbols

echo "Running dylib tests with a weak symbol"

apple g++ -g -fPIC -dynamiclib mach/dylib/weak_lib.cc -o mach/dylib/weak_lib.dylib
apple g++ -g -fPIC mach/dylib/weak_main.cc -o mach/dylib/weak_main mach/dylib/weak_lib.dylib
apple g++ -g -fPIC mach/dylib/weak_main.cc -o mach/dylib/weak_main-dl -DDL

echo "Running mach/dylib/weak_main"
./ld-mac mach/dylib/weak_main

echo "Running mach/dylib/weak_main-dl"
./ld-mac mach/dylib/weak_main-dl

# Compile and run unit tests with clang

# Need this file from Xcode 4
CLANG=$MAC_BIN_DIR/clang-137
if [ -x $CLANG ]; then
    for i in mach/*.c; do
        echo "Compiling $i with clang"
        ./ld-mac $CLANG -Wl,-syslibroot,$MAC_TOOL_DIR -isysroot $MAC_TOOL_DIR $i -o $i.clang.bin
        echo "Running $i.clang.bin"
        ./ld-mac $i.clang.bin
    done
else
    echo "$CLANG not found, skipping some tests with it"
fi

echo
echo '*** ALL TESTS PASS ***'
