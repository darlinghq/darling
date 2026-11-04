#!/bin/sh
# Builds the real commpage.c with a test driver and checks commpage_setup(): it must refuse to replace an
# existing mapping in the commpage range, and must leave the data page read-only and the text part read+exec.
# Runs on x86_64 (32-page range with a text part) and aarch64 (single page); the text checks need x86_64.
# usage: [MLDR_DIR=dir-with-commpage.c] [XNU_CAPS_INCLUDE=dir-containing-i386/cpu_capabilities.h] commpage_test.sh [scratch-dir]
# XNU_CAPS_INCLUDE is needed on x86_64 when src/external/xnu is not checked out (the header in mldr/include is a symlink into it).
set -e
here=$(cd "$(dirname "$0")" && pwd)
dir=${MLDR_DIR:-$here/..}
out=${1:-${TMPDIR:-.}/commpage_test}
mkdir -p "$out"
caps=${XNU_CAPS_INCLUDE:+-I$XNU_CAPS_INCLUDE}
${CC:-cc} -O0 -g -Wall $caps -I"$dir/include" -I"$dir" -o "$out/test" "$here/commpage_test.c" "$dir/commpage.c"
"$out/test"
