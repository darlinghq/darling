#!/bin/sh
# DiskArbitration with disk images, against the behaviour observed on macOS (see diskarbitration.md).
# Needs DARLING_SKILLS (a darling-skills checkout) and a running Darling.
set -u

here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
gw="/Volumes/SystemRoot$work"
failures=0

pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; failures=$((failures + 1)); }

# runs a shell command line in the guest; stdout, stderr and exit status go to $work/<name>.{out,err,rc}
# (a command that uses exit must run in a subshell, or no status is written)
guest() {
	name=$1
	shift
	timeout -s KILL 300 darling shell /bin/sh -c "{ $1; } >'$gw/$name.out' 2>'$gw/$name.err'; echo \$? >'$gw/$name.rc'" </dev/null >/dev/null 2>&1
}
rc() { cat "$work/$1.rc" 2>/dev/null; }
outputs() { cat "$work/$1.out" "$work/$1.err" 2>/dev/null | tr '\n' '|'; }

build() { # <program> <source>
	"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/$1" "$2" \
		-framework DiskArbitration -framework CoreFoundation >"$work/$1.build" 2>&1 \
		|| { fail "$1 does not build: $(cat "$work/$1.build")"; return 1; }
}

cp "$here/fixtures/unmount-test.dmg" "$here/fixtures/slash-name.dmg" "$here/fixtures/amp-name.dmg" "$work/"

# the existing framework checks still hold
for t in diskarbitration_match diskarbitration_callbacks; do
	if build "$t" "$here/src/$t.c"; then
		guest "$t" "'$gw/$t'"
		[ "$(rc "$t")" = 0 ] && pass "$t" || fail "$t: $(outputs "$t")"
	fi
done

build da-disks "$here/da-disks.c" || { rm -rf "$work"; exit 1; }

# lookups and descriptions
guest attach "hdiutil attach -quiet '$gw/unmount-test.dmg' && hdiutil attach -quiet '$gw/slash-name.dmg' && hdiutil attach -quiet '$gw/amp-name.dmg'"
[ "$(rc attach)" = 0 ] || fail "attach: $(outputs attach)"
guest disks "'$gw/da-disks' '/Volumes/Unmount Test' 'Unmount Test' '/Volumes/A:B' 'A/B' '/Volumes/Tom & Jerry <1>' 'Tom & Jerry <1>'"
cat "$work/disks.out" "$work/disks.err"
[ "$(rc disks)" = 0 ] && pass "disk lookups and descriptions" || fail "disk lookups and descriptions"
guest detach "hdiutil detach -quiet '/Volumes/A:B'; hdiutil detach -quiet '/Volumes/Tom & Jerry <1>'"

# a mount point in /Volumes goes away with its volume, even after a bare unmount(2) (macOS does the same)
guest unmount "'$gw/da-disks' --unmount '/Volumes/Unmount Test'"
guest gone "(for i in 1 2 3 4 5 6 7 8 9 10; do test -e '/Volumes/Unmount Test' || exit 0; sleep 0.5; done; exit 1)"
[ "$(rc unmount)" = 0 ] && [ "$(rc gone)" = 0 ] && pass "mount point removed after unmount(2)" \
	|| fail "mount point removed after unmount(2): unmount $(rc unmount) $(outputs unmount), still there: $(rc gone)"

# removing old mount points never takes one an attach has just made for a new volume of the same name
guest rounds "(for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
	mp=\$(hdiutil attach '$gw/unmount-test.dmg' | tail -n 1 | cut -f 3) || exit 1
	test \"\$mp\" = '/Volumes/Unmount Test' || { echo \"round \$i: mounted at \$mp\"; exit 1; }
	cat '/Volumes/Unmount Test/hello.txt' >/dev/null || { echo \"round \$i: not readable\"; exit 1; }
	hdiutil detach -quiet '/Volumes/Unmount Test' || { echo \"round \$i: detach failed\"; exit 1; }
done)"
[ "$(rc rounds)" = 0 ] && pass "unmount then attach at once, 20 rounds" || fail "unmount then attach at once: $(outputs rounds)"

# disk appeared, disappeared and description-changed callbacks (see da-callbacks.c; a daemon restart is
# diskarbitration-restart.sh)
if build da-callbacks "$here/da-callbacks.c"; then
	guest cb-attach "hdiutil attach -quiet '$gw/unmount-test.dmg'"
	guest cb-existing "'$gw/da-callbacks' existing '/Volumes/Unmount Test'"
	guest cb-detach "hdiutil detach -quiet '/Volumes/Unmount Test'"
	guest cb-attach-detach "'$gw/da-callbacks' attach-detach '$gw/unmount-test.dmg' '/Volumes/Unmount Test'"
	guest cb-unscheduled "'$gw/da-callbacks' unscheduled"
	guest cb-unregister "'$gw/da-callbacks' unregister"
	for t in existing attach-detach unscheduled unregister; do
		[ "$(rc "cb-$t")" = 0 ] && pass "callbacks: $t" || fail "callbacks: $t: $(outputs "cb-$t")"
	done
fi

rm -rf "$work"
[ "$failures" = 0 ]
