#!/bin/bash
#
# Reindent CWD recursively using clang-format (assumed to be in your PATH),
# and per-component or per-directory .clang-format style specifications.
#

CPUS=`sysctl -n hw.logicalcpu`
CLANGFORMAT=${CLANGFORMAT:=`xcrun -find clang-format`}

if [ ! -x "${CLANGFORMAT}" ]; then
    echo "Could not find clang-format" 1>&2
    exit 1
fi

echo "Using ${CLANGFORMAT} to reindent, using concurrency of ${CPUS}"

find -x . \! \( \( -name BUILD -o -name EXTERNAL_HEADERS -o -name libMicro -o -name zlib -o -name .svn -o -name .git -o -name cscope.\* -o -name \*~ \) -prune \) -type f \( -name \*.c -o -name \*.cpp \) -print0 | \
    xargs -0 -P "${CPUS}" -n 10 "${CLANGFORMAT}" -style=file -i
ret=$?

if [ $ret -ne 0 ]; then
    echo "reindent failed: $ret" 1>&2
    exit 1
fi

exit 0

