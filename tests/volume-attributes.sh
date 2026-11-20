#!/bin/sh
# getattrlist(ATTR_VOL_NAME) and statfs().f_fstypename for disk image volumes: a volume keeps its own name when its
# mount point carries a " 1" suffix or shows "/" as ":", and its kind is "hfs" (as on macOS). Also statfs().f_fsid of
# each kind of volume. Runs darling-xnu's darling/tests/volume-name-kind.c and statfs-fsid.c in the guest.
# Needs DARLING_SKILLS (a darling-skills checkout) and a running Darling.
set -u

here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
gw="/Volumes/SystemRoot$work"
failures=0

pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; failures=$((failures + 1)); }

guest() {
	name=$1
	shift
	timeout -s KILL 120 darling shell /bin/sh -c "{ $1; } >'$gw/$name.out' 2>'$gw/$name.err'; echo \$? >'$gw/$name.rc'" </dev/null >/dev/null 2>&1
}

cp "$here/fixtures/unmount-test.dmg" "$work/first.dmg"
cp "$here/fixtures/unmount-test.dmg" "$work/second.dmg"
cp "$here/fixtures/slash-name.dmg" "$work/"

"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/volume-name-kind" \
	"$here/../src/external/xnu/darling/tests/volume-name-kind.c" || { echo "FAIL: volume-name-kind does not build"; exit 1; }
"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/statfs-fsid" \
	"$here/../src/external/xnu/darling/tests/statfs-fsid.c" || { echo "FAIL: statfs-fsid does not build"; exit 1; }

guest attach "hdiutil attach -quiet '$gw/first.dmg' && hdiutil attach -quiet '$gw/second.dmg' && hdiutil attach -quiet '$gw/slash-name.dmg'"
[ "$(cat "$work/attach.rc")" = 0 ] || fail "attach: $(cat "$work/attach.err")"

run() { # <case> <mount point> <name> <kind> [<file>]
	case_name=$1
	shift
	guest "$case_name" "'$gw/volume-name-kind' '$1' '$2' '$3' ${4:+'$4'}"
	[ "$(cat "$work/$case_name.rc")" = 0 ] && pass "$case_name" \
		|| fail "$case_name: $(cat "$work/$case_name.out" "$work/$case_name.err" | tr '\n' '|')"
}

run "volume name and kind" "/Volumes/Unmount Test" "Unmount Test" hfs "/Volumes/Unmount Test/hello.txt"
run "name of a volume mounted with a suffix" "/Volumes/Unmount Test 1" "Unmount Test" hfs
run "name with a slash" "/Volumes/A:B" "A/B" hfs

# f_fsid: the volume's device number on each volume (the guest's root and home, the host's file system, an image)
guest fsid "'$gw/statfs-fsid' / \"\$HOME\" /Volumes/SystemRoot '$gw' '/Volumes/Unmount Test'"
[ "$(cat "$work/fsid.rc")" = 0 ] && pass "f_fsid of each volume" \
	|| fail "f_fsid of each volume: $(grep -v '^PASS' "$work/fsid.out" "$work/fsid.err" | tr '\n' '|')"

guest detach "hdiutil detach -quiet '/Volumes/Unmount Test 1'; hdiutil detach -quiet '/Volumes/Unmount Test'; hdiutil detach -quiet '/Volumes/A:B'"

rm -rf "$work"
[ "$failures" = 0 ]
