#!/bin/sh
# hdiutil attach and the libdarling-dmg API against the behaviour observed on macOS (see dmg-attach.md),
# then unmount-disk-image.sh as a regression.
# Needs DARLING_SKILLS (a darling-skills checkout), DARLING_ROOT (the installed tree) and a running Darling.
set -u

here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
gw="/Volumes/SystemRoot$work"
image="$gw/Unmount Test.dmg"
second="$gw/Second Copy.dmg"
third="$gw/Third Copy.dmg"
fourth="$gw/Fourth Copy.dmg"
renamed="$gw/renamed.dmg"
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

volumes_and_helpers() {
	guest "$1" "ls /Volumes; ps ax | grep '[d]arling-dmg-mount'"
}

cp "$here/fixtures/unmount-test.dmg" "$work/Unmount Test.dmg"
for f in "Second Copy" "Third Copy" "Fourth Copy" renamed; do cp "$here/fixtures/unmount-test.dmg" "$work/$f.dmg"; done
cp "$here/fixtures/slash-name.dmg" "$work/slash-name.dmg"
echo "this is not a disk image" > "$work/text.txt"
mkdir "$work/mountroot"

# a missing file
guest missing "hdiutil attach '$gw/nonexistent.dmg'"
[ "$(rc missing)" = 1 ] && [ "$(cat "$work/missing.err")" = "hdiutil: attach failed - No such file or directory" ] \
	&& pass "attach of a missing file" || fail "attach of a missing file: rc $(rc missing), stderr: $(cat "$work/missing.err")"

# a file that is not a disk image: refused, and nothing left behind
volumes_and_helpers before
guest text "hdiutil attach '$gw/text.txt'"
volumes_and_helpers after
[ "$(rc text)" = 1 ] && [ "$(cat "$work/text.err")" = "hdiutil: attach failed - image not recognized" ] \
	&& pass "attach of a non-image" || fail "attach of a non-image: rc $(rc text), stderr: $(cat "$work/text.err")"
cmp -s "$work/before.out" "$work/after.out" \
	&& pass "attach of a non-image leaves no directory or helper behind" \
	|| fail "attach of a non-image left something behind: $(diff "$work/before.out" "$work/after.out" | tr '\n' ' ')"

# -plist names the mount point
guest plist "hdiutil attach -plist '$image'"
mp=$(grep -A1 '<key>mount-point</key>' "$work/plist.out" | sed -n 's:.*<string>\(.*\)</string>.*:\1:p')
[ "$(rc plist)" = 0 ] && [ "$mp" = "/Volumes/Unmount Test" ] \
	&& pass "attach -plist reports the mount point" || fail "attach -plist: rc $(rc plist), mount point '$mp', stderr: $(cat "$work/plist.err")"
guest d1 "hdiutil detach -quiet '$mp'"

# the volume outlives the process that attached it
guest attachq "hdiutil attach -quiet '$image'"
guest read "cat '/Volumes/Unmount Test/hello.txt'"
[ "$(rc attachq)" = 0 ] && [ "$(cat "$work/read.out")" = hello ] \
	&& pass "attach survives the caller" || fail "attach survives the caller: rc $(rc attachq), read: $(cat "$work/read.out" "$work/read.err")"

# a second image with the same volume name at the same time: the lowest free " <n>" suffix, from 1, in /Volumes
guest second "hdiutil attach '$second'"
mp2=$(tail -n 1 "$work/second.out")
guest read2 "cat '$mp2/hello.txt' && cat '/Volumes/Unmount Test/hello.txt'"
[ "$(rc second)" = 0 ] && [ "$mp2" = "/Volumes/Unmount Test 1" ] && [ "$(cat "$work/read2.out")" = "hello
hello" ] && pass "two images attached at once" || fail "two images: rc $(rc second), mount point '$mp2', read: $(cat "$work/read2.out" "$work/read2.err")"
guest third "hdiutil attach '$third'"
[ "$(tail -n 1 "$work/third.out")" = "/Volumes/Unmount Test 2" ] && pass "third image with the same name" \
	|| fail "third image: $(cat "$work/third.out" "$work/third.err")"
guest fourth "hdiutil detach -quiet '/Volumes/Unmount Test 1' && hdiutil attach '$fourth'"
[ "$(tail -n 1 "$work/fourth.out")" = "/Volumes/Unmount Test 1" ] && pass "a freed name is used again" \
	|| fail "freed name: $(cat "$work/fourth.out" "$work/fourth.err")"
guest d2 "for v in '/Volumes/Unmount Test 1' '/Volumes/Unmount Test 2' '/Volumes/Unmount Test'; do hdiutil detach -quiet \"\$v\"; done"

# the mount point is named after the volume, not the file
guest renamed "hdiutil attach '$renamed'"
[ "$(tail -n 1 "$work/renamed.out")" = "/Volumes/Unmount Test" ] && pass "mount point named after the volume" \
	|| fail "renamed image: $(cat "$work/renamed.out" "$work/renamed.err")"
guest d3 "hdiutil detach -quiet '/Volumes/Unmount Test'"

# a "/" in the volume name appears as ":" in the mount point
guest slash "hdiutil attach '$gw/slash-name.dmg'"
[ "$(tail -n 1 "$work/slash.out")" = "/Volumes/A:B" ] && pass "slash in the volume name" \
	|| fail "slash in the volume name: $(cat "$work/slash.out" "$work/slash.err")"
guest d4 "hdiutil detach -quiet '/Volumes/A:B'"

# under another mount root the suffix for a taken name starts at 2
mkdir "$work/root2"
guest mr1 "hdiutil attach -mountroot '$gw/root2' '$image'"
guest mr2 "hdiutil attach -mountroot '$gw/root2' '$second'"
[ "$(tail -n 1 "$work/mr1.out")" = "$gw/root2/Unmount Test" ] && [ "$(tail -n 1 "$work/mr2.out")" = "$gw/root2/Unmount Test 2" ] \
	&& pass "same name under -mountroot" || fail "same name under -mountroot: $(cat "$work/mr1.out" "$work/mr1.err" "$work/mr2.out" "$work/mr2.err")"
guest d5 "hdiutil detach -quiet '$gw/root2/Unmount Test 2'; hdiutil detach -quiet '$gw/root2/Unmount Test'"

# the library API
"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/dmg-api" "$here/dmg-api.c" \
	-I"$here/../src/external/darling-dmg/include" -L"$DARLING_ROOT/usr/lib/darling" -ldarling-dmg \
	&& { guest api "'$gw/dmg-api' '$renamed' 'Unmount Test' '$gw/text.txt' '$gw/mountroot'"; cat "$work/api.out" "$work/api.err"
	     [ "$(rc api)" = 0 ] && pass "library API" || fail "library API"; } \
	|| fail "library API test does not build"

"$here/unmount-disk-image.sh" || failures=$((failures + 1))

rm -rf "$work"
[ "$failures" = 0 ]
