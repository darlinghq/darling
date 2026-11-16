#!/bin/sh
# Licence agreements in disk images: the libdarling-dmg API (dmg-license.c) and hdiutil attach (see dmg-license.md).
# Needs DARLING_SKILLS (a darling-skills checkout), DARLING_ROOT (the installed tree) and a running Darling.
set -u

here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
gw="/Volumes/SystemRoot$work"
failures=0

pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; failures=$((failures + 1)); }

# runs a shell command line in the guest; stdout, stderr and exit status go to $work/<name>.{out,err,rc}
guest() {
	name=$1
	shift
	timeout -s KILL 120 darling shell /bin/sh -c "{ $1; } >'$gw/$name.out' 2>'$gw/$name.err'; echo \$? >'$gw/$name.rc'" </dev/null >/dev/null 2>&1
}
rc() { cat "$work/$1.rc" 2>/dev/null; }

for f in sla-test sla-short-lpic sla-overcount unmount-test; do cp "$here/fixtures/$f.dmg" "$work/$f.dmg"; done

"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/dmg-license" "$here/dmg-license.c" \
	-I"$here/../src/external/darling-dmg/include" -L"$DARLING_ROOT/usr/lib/darling" -ldarling-dmg \
	&& { guest api "'$gw/dmg-license' '$gw/sla-test.dmg' '$gw/unmount-test.dmg' '$gw/sla-short-lpic.dmg' '$gw/sla-overcount.dmg'"
	     cat "$work/api.out" "$work/api.err"
	     [ "$(rc api)" = 0 ] && pass "licence agreement API" || fail "licence agreement API"; } \
	|| fail "licence agreement API test does not build"

rm -rf "$work"
[ "$failures" = 0 ]
