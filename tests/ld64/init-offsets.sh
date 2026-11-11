#!/bin/sh
# Copyright (C) 2026 Lubos Dolezel
#
# Host test for ld64: -fixup_chains -init_offsets links, and
# __TEXT,__init_offsets holds the image offsets of the initializers.
# Usage: init-offsets.sh <ld64 binary>
# Needs clang and llvm-objdump. Exit 0 on pass, 1 on fail.
set -eu
ld=$1
here=$(cd "$(dirname "$0")" && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
failures=0
for arch in arm64 x86_64; do
	clang -target $arch-apple-macos11 -O1 -c "$here/init-offsets.c" -o "$tmp/i.o"
	if ! "$ld" -arch $arch -dylib -platform_version macos 11.0 11.0 -fixup_chains -init_offsets -o "$tmp/i.dylib" "$tmp/i.o" 2>"$tmp/err"; then
		echo "FAIL: $arch: link failed: $(grep -v 'directory not found' "$tmp/err")"
		failures=$((failures + 1)); continue
	fi
	# a dylib's mach_header is at address 0, so the offsets are the function addresses
	expected=$(for a in $(llvm-objdump --macho -t "$tmp/i.dylib" | awk '$NF == "_init1" || $NF == "_init2" { print $1 }'); do printf '%d ' "0x$a"; done)
	got=$(python3 -I -c '
import struct, sys
d = open(sys.argv[1], "rb").read()
ncmds = struct.unpack_from("<I", d, 16)[0]
off = 32
for _ in range(ncmds):
    cmd, size = struct.unpack_from("<2I", d, off)
    if cmd == 0x19:  # LC_SEGMENT_64
        for i in range(struct.unpack_from("<I", d, off + 64)[0]):
            s = off + 72 + i * 80
            if d[s:s + 16].rstrip(b"\0") == b"__init_offsets":
                size64, foff = struct.unpack_from("<QI", d, s + 40)
                print(" ".join(str(v) for v in struct.unpack_from("<%dI" % (size64 // 4), d, foff)), end=" ")
    off += size
' "$tmp/i.dylib")
	if [ -n "$expected" ] && [ "$got" = "$expected" ]; then
		echo "PASS: $arch"
	else
		echo "FAIL: $arch: __init_offsets is '$got', expected '$expected'"; failures=$((failures + 1))
	fi
done
[ $failures = 0 ]
