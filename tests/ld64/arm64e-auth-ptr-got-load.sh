#!/bin/sh
# Copyright (C) 2026 Lubos Dolezel
#
# Host test for ld64: GOT loads of an arm64e __auth_ptr slot stay loads.
# Usage: arm64e-auth-ptr-got-load.sh <ld64 binary>
# Needs clang (arm64e assembler) and llvm-objdump. Exit 0 on pass, 1 on fail.
set -eu
ld=$1
here=$(cd "$(dirname "$0")" && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
clang -target arm64e-apple-macos11 -c "$here/arm64e-auth-ptr-got-load.s" -o "$tmp/t.o"
"$ld" -arch arm64e -dylib -platform_version macos 11.0 11.0 -o "$tmp/t.dylib" "$tmp/t.o" 2>/dev/null
second=$(llvm-objdump --macho -d "$tmp/t.dylib" | sed -n '/^_get:/{n;n;p;q}')
case $second in
*ldr*x0,*) echo "PASS: $second" ;;
*) echo "FAIL: expected an LDR from the __auth_ptr slot, got: $second"; exit 1 ;;
esac
