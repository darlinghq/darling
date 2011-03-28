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

for i in mach/*.bin; do
    echo "Running $i"
    ./ld-mac ./$i
done

MAC_TOOL_DIR=/usr/i686-apple-darwin10
MAC_BIN_DIR=$MAC_TOOL_DIR/usr/bin

echo "Running gcc -c mach/hello.c"
./ld-mac $MAC_BIN_DIR/gcc -c mach/hello.c
echo "Running gcc mach/hello.c"
./ld-mac $MAC_BIN_DIR/gcc mach/hello.c
echo "Running gcc -g mach/hello.c"
PATH=$MAC_BIN_DIR ./ld-mac $MAC_BIN_DIR/gcc -g mach/hello.c

echo "Running clang -c mach/hello.c"
./ld-mac $MAC_BIN_DIR/clang -c mach/hello.c

# Need this file from Xcode 4
CLANG=$MAC_BIN_DIR/clang-137
if [ -x $CLANG ]; then
    for i in mach/*.c; do
        echo "Compiling $i with clang"
        ./ld-mac $CLANG -Wl,-syslibroot,$MAC_TOOL_DIR -isysroot $MAC_TOOL_DIR $i -o $i.clang.bin
        echo "Running $i.clang.bin"
        $i.clang.bin
    done
else
    echo "$CLANG not found, skipping some tests with it"
fi

echo
echo '*** ALL TESTS PASS ***'
