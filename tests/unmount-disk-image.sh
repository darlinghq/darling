#!/bin/sh
# Mounts tests/fixtures/unmount-test.dmg in Darling and checks unmount(2) (unmount-disk-image.c) and
# hdiutil detach against the behaviour observed on macOS (see unmount-disk-image.md).
# Needs DARLING_SKILLS pointing at a darling-skills checkout, and a running Darling.
set -u

here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
guest_work="/Volumes/SystemRoot$work"
image="$guest_work/Unmount Test.dmg"
mp="/Volumes/Unmount Test"
failures=0

pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; failures=$((failures + 1)); }

# runs a shell command line in the guest, its stdout, stderr and exit status in $work/<name>.{out,err,rc}
guest() {
	name=$1
	shift
	timeout -s KILL 120 darling shell /bin/sh -c "{ $1; } >'$guest_work/$name.out' 2>'$guest_work/$name.err'; echo \$? >'$guest_work/$name.rc'" </dev/null >/dev/null 2>&1
}

attach() {
	guest attach "hdiutil attach '$image'"
	[ "$(cat "$work/attach.rc" 2>/dev/null)" = 0 ] || { fail "hdiutil attach: $(cat "$work/attach.err" 2>/dev/null)"; return 1; }
}

"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/unmount-disk-image" "$here/unmount-disk-image.c" || exit 1
cp "$here/fixtures/unmount-test.dmg" "$work/Unmount Test.dmg"

# unmount(2)
if attach; then
	guest prog "'$guest_work/unmount-disk-image' '$mp'"
	cat "$work/prog.out"
	[ "$(cat "$work/prog.rc")" = 0 ] && pass "unmount(2) checks" || fail "unmount(2) checks"
	guest cleanup1 "hdiutil detach -force '$mp'"
fi

# unmount(2) again in a program that ignores SIGCHLD (its children are reaped by the kernel)
if attach; then
	guest prog2 "'$guest_work/unmount-disk-image' '$mp' ignore-sigchld"
	cat "$work/prog2.out"
	[ "$(cat "$work/prog2.rc")" = 0 ] && pass "unmount(2) checks, SIGCHLD ignored" || fail "unmount(2) checks, SIGCHLD ignored"
	guest cleanup2 "hdiutil detach -force '$mp'"
fi

# hdiutil detach of a busy volume: refused with EBUSY's message and exit status
if attach; then
	guest busy "exec 3<'$mp/hello.txt'; hdiutil detach '$mp'"
	grep -q "^hdiutil: couldn't unmount \".*\" - Resource busy\$" "$work/busy.err" && [ "$(cat "$work/busy.rc")" = 16 ] \
		&& pass "busy detach refused (exit 16)" \
		|| fail "busy detach: rc $(cat "$work/busy.rc"), stderr: $(cat "$work/busy.err")"
	guest busyq "exec 3<'$mp/hello.txt'; hdiutil detach -quiet '$mp'"
	[ ! -s "$work/busyq.err" ] && [ "$(cat "$work/busyq.rc")" = 16 ] \
		&& pass "busy detach -quiet is silent (exit 16)" \
		|| fail "busy detach -quiet: rc $(cat "$work/busyq.rc"), stderr: $(cat "$work/busyq.err")"

	# -force detaches it anyway
	guest force "exec 3<'$mp/hello.txt'; hdiutil detach -force '$mp'; rc=\$?; test -e '$mp' && echo still-there >&2; (exit \$rc)"
	[ "$(cat "$work/force.rc")" = 0 ] && grep -q '^".*" ejected\.$' "$work/force.out" && ! grep -q still-there "$work/force.err" \
		&& pass "detach -force of a busy volume" \
		|| fail "detach -force: rc $(cat "$work/force.rc"), out: $(cat "$work/force.out"), err: $(cat "$work/force.err")"
fi

# plain detach
if attach; then
	guest plain "hdiutil detach '$mp'; rc=\$?; test -e '$mp' && echo still-there >&2; (exit \$rc)"
	[ "$(cat "$work/plain.rc")" = 0 ] && grep -q '^".*" ejected\.$' "$work/plain.out" && ! grep -q still-there "$work/plain.err" \
		&& pass "detach" \
		|| fail "detach: rc $(cat "$work/plain.rc"), out: $(cat "$work/plain.out"), err: $(cat "$work/plain.err")"
fi

# a path that is not a mounted disk image
guest missing "hdiutil detach /Volumes/nonexistent-unmount-test"
[ "$(cat "$work/missing.rc")" = 1 ] && grep -q '^hdiutil: detach failed - No such file or directory$' "$work/missing.err" \
	&& pass "detach of a missing path" \
	|| fail "detach of a missing path: rc $(cat "$work/missing.rc"), stderr: $(cat "$work/missing.err")"

rm -rf "$work"
[ "$failures" = 0 ]
