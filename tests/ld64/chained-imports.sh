#!/bin/sh
# Copyright (C) 2026 Lubos Dolezel
#
# Host test for ld64: the library ordinals in the chained-fixup import table
# (-fixup_chains) follow the same rules as opcode-based binds:
# -undefined dynamic_lookup, -U and flat-namespace output import with
# BIND_SPECIAL_DYLIB_FLAT_LOOKUP (-2), and two-level imports from a linked
# dylib use its ordinal (1).
# Usage: chained-imports.sh <ld64 binary>
# Needs clang (arm64 target) and python3. Exit 0 on pass, 1 on fail.
set -eu
ld=$1
here=$(cd "$(dirname "$0")" && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
clang -target arm64-apple-macos11 -O1 -c "$here/chained-imports-undefined.c" -o "$tmp/u.o"
clang -target arm64-apple-macos11 -O1 -c "$here/chained-imports-dep.c" -o "$tmp/dep.o"
link() { "$ld" -arch arm64 -dylib -platform_version macos 11.0 11.0 -fixup_chains "$@" 2>/dev/null; }
link -install_name @rpath/libdep.dylib -o "$tmp/libdep.dylib" "$tmp/dep.o"
failures=0
check() {
	what=$1; expected=$2; shift 2
	if ! link -o "$tmp/out.dylib" "$tmp/u.o" "$@"; then
		echo "FAIL: $what: link failed"; failures=$((failures + 1)); return
	fi
	got=$(python3 -I "$here/chained-imports.py" "$tmp/out.dylib" | awk '{print $2}' | sort -u | tr '\n' ' ')
	if [ "$got" = "$expected " ]; then
		echo "PASS: $what"
	else
		echo "FAIL: $what: ordinals $got, expected $expected"; failures=$((failures + 1))
	fi
}
for i in 1 2 3; do
	check "-undefined dynamic_lookup (link $i)" -2 -undefined dynamic_lookup
done
check "-U" -2 -U _undefined_elsewhere -U _also_undefined
check "-flat_namespace -undefined suppress" -2 -flat_namespace -undefined suppress
check "-flat_namespace with a dylib" -2 -flat_namespace "$tmp/libdep.dylib"
check "two-level with a dylib" 1 "$tmp/libdep.dylib"
[ $failures = 0 ]
