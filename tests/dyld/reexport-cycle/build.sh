#!/bin/sh
# Builds reexport-cycle and its libraries in the current directory.
# Usage: build.sh <SDK> <directory with the ld64 "ld" symlink> [clang target]
set -eu
here=$(cd "$(dirname "$0")" && pwd)
sdk=$1; ldDir=$2; target=${3:-x86_64-apple-macos10.15}
cc="clang -target $target -isysroot $sdk -B $ldDir -D__DARWIN_ONLY_UNIX_CONFORMANCE=1"
lib() { name=$1; src=$2; shift 2; $cc -dynamiclib -install_name "@rpath/$name.dylib" -o "$name.dylib" "$here/$src" "$@"; }
# the cycle needs librxcyclea to exist before librxcycleb can re-export it
lib librxcyclea reexport-cycle-anchor.c
lib librxcycleb reexport-cycle-anchor.c -Wl,-reexport_library,librxcyclea.dylib
lib librxcycleleaf reexport-cycle-leaf.c
lib librxcyclea reexport-cycle-anchor.c -Wl,-reexport_library,librxcycleb.dylib -Wl,-reexport_library,librxcycleleaf.dylib
$cc -o reexport-cycle "$here/reexport-cycle.c" -Wl,-rpath,@executable_path
