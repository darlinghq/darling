#!/bin/sh
# Runs on the Linux host, not in Darling. A darling-dmg FUSE volume names itself to Darling's kernel layer through
# two xattrs on its root: darling.volume.name (the HFS+ volume name, UTF-8) and darling.volume.kind ("hfs"). They are
# not listed, exist only on the root, and guests never see them (Darling maps guest xattrs to "user." ones).
# Needs cmake, a C++ compiler, the FUSE 2 development files, fusermount and getfattr.
set -u

here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
failures=0

pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; failures=$((failures + 1)); }

cmake -S "$here/../src/external/darling-dmg" -B "$work/build" >"$work/cmake.log" 2>&1 \
	&& make -C "$work/build" darling-dmg >"$work/make.log" 2>&1 \
	|| { echo "FAIL: darling-dmg does not build (see $work)"; exit 1; }

check() { # <image> <expected name>
	mp="$work/mp"
	mkdir -p "$mp"
	if ! "$work/build/darling-dmg" "$here/fixtures/$1" "$mp" >"$work/mount.log" 2>&1; then
		fail "$1: does not mount: $(cat "$work/mount.log")"
		return
	fi

	name=$(getfattr --only-values -n darling.volume.name "$mp" 2>"$work/err")
	[ "$name" = "$2" ] && pass "$1: volume name" || fail "$1: volume name '$name' $(cat "$work/err")"
	kind=$(getfattr --only-values -n darling.volume.kind "$mp" 2>"$work/err")
	[ "$kind" = hfs ] && pass "$1: volume kind" || fail "$1: volume kind '$kind' $(cat "$work/err")"
	getfattr -d -m - "$mp" 2>/dev/null | grep -q "darling\." && fail "$1: the attributes are listed" \
		|| pass "$1: the attributes are not listed"
	first=$(ls "$mp" | head -n 1)
	if [ -n "$first" ]; then
		getfattr -n darling.volume.name "$mp/$first" >/dev/null 2>&1 && fail "$1: a file inside has a volume name" \
			|| pass "$1: only the root has a volume name"
	fi

	fusermount -u "$mp"
}

check unmount-test.dmg "Unmount Test"
check slash-name.dmg "A/B"
check amp-name.dmg "Tom & Jerry <1>"

rm -rf "$work"
[ "$failures" = 0 ]
