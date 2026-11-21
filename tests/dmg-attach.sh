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
# the mount point attach printed: the last field of the last line of its table
mount_point() { tail -n 1 "$work/$1.out" | cut -f 3; }

volumes_and_helpers() {
	guest "$1" "ls /Volumes; ps ax | grep '[d]arling-dmg-mount'"
}

cp "$here/fixtures/unmount-test.dmg" "$work/Unmount Test.dmg"
for f in "Second Copy" "Third Copy" "Fourth Copy" renamed; do cp "$here/fixtures/unmount-test.dmg" "$work/$f.dmg"; done
cp "$here/fixtures/slash-name.dmg" "$here/fixtures/amp-name.dmg" "$here/fixtures/bad-blkx.dmg" "$work/"
gunzip -c "$here/fixtures/long-name-key.hfs.gz" > "$work/long-name-key.hfs"
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

# attaching an attached image gives its mount point again, mounting nothing more (macOS 26.7.3, see dmg-attach.md)
guest twice1 "hdiutil attach '$image'"
guest twice2 "hdiutil attach '$image'"
guest twicecount "for d in '/Volumes/Unmount Test'*; do [ \"\$(stat -f %d \"\$d\")\" != \"\$(stat -f %d /Volumes)\" ] && echo \"\$d\"; done | wc -l | tr -d ' '"
[ "$(rc twice1)" = 0 ] && [ "$(rc twice2)" = 0 ] && [ "$(mount_point twice1)" = "/Volumes/Unmount Test" ] \
	&& [ "$(mount_point twice2)" = "/Volumes/Unmount Test" ] && [ "$(cat "$work/twicecount.out")" = 1 ] \
	&& pass "attach twice" || fail "attach twice: rc $(rc twice1) $(rc twice2), $(cat "$work/twice2.out" "$work/twice2.err" "$work/twicecount.out" | tr '\n' '|')"
guest twiced "hdiutil detach -quiet '/Volumes/Unmount Test'"

# the volume outlives the process that attached it
guest attachq "hdiutil attach -quiet '$image'"
guest read "cat '/Volumes/Unmount Test/hello.txt'"
[ "$(rc attachq)" = 0 ] && [ "$(cat "$work/read.out")" = hello ] \
	&& pass "attach survives the caller" || fail "attach survives the caller: rc $(rc attachq), read: $(cat "$work/read.out" "$work/read.err")"

# a second image with the same volume name at the same time: the lowest free " <n>" suffix, from 1, in /Volumes
guest second "hdiutil attach '$second'"
mp2=$(mount_point second)
guest read2 "cat '$mp2/hello.txt' && cat '/Volumes/Unmount Test/hello.txt'"
[ "$(rc second)" = 0 ] && [ "$mp2" = "/Volumes/Unmount Test 1" ] && [ "$(cat "$work/read2.out")" = "hello
hello" ] && pass "two images attached at once" || fail "two images: rc $(rc second), mount point '$mp2', read: $(cat "$work/read2.out" "$work/read2.err")"
guest third "hdiutil attach '$third'"
[ "$(mount_point third)" = "/Volumes/Unmount Test 2" ] && pass "third image with the same name" \
	|| fail "third image: $(cat "$work/third.out" "$work/third.err")"
guest fourth "hdiutil detach -quiet '/Volumes/Unmount Test 1' && hdiutil attach '$fourth'"
[ "$(mount_point fourth)" = "/Volumes/Unmount Test 1" ] && pass "a freed name is used again" \
	|| fail "freed name: $(cat "$work/fourth.out" "$work/fourth.err")"
guest d2 "for v in '/Volumes/Unmount Test 1' '/Volumes/Unmount Test 2' '/Volumes/Unmount Test'; do hdiutil detach -quiet \"\$v\"; done"

# the mount point is named after the volume, not the file
guest renamed "hdiutil attach '$renamed'"
[ "$(mount_point renamed)" = "/Volumes/Unmount Test" ] && pass "mount point named after the volume" \
	|| fail "renamed image: $(cat "$work/renamed.out" "$work/renamed.err")"
guest d3 "hdiutil detach -quiet '/Volumes/Unmount Test'"

# a "/" in the volume name appears as ":" in the mount point
guest slash "hdiutil attach '$gw/slash-name.dmg'"
[ "$(mount_point slash)" = "/Volumes/A:B" ] && pass "slash in the volume name" \
	|| fail "slash in the volume name: $(cat "$work/slash.out" "$work/slash.err")"
guest d4 "hdiutil detach -quiet '/Volumes/A:B'"

# two images with the same volume name attached at the same moment: each gets its own mount point
guest parallel "hdiutil attach '$image' >'$gw/p1.out' & hdiutil attach '$second' >'$gw/p2.out' & wait; { tail -n 1 '$gw/p1.out'; tail -n 1 '$gw/p2.out'; } | cut -f 3 | sort; cat '/Volumes/Unmount Test/hello.txt' '/Volumes/Unmount Test 1/hello.txt'"
[ "$(cat "$work/parallel.out")" = "/Volumes/Unmount Test
/Volumes/Unmount Test 1
hello
hello" ] && pass "same name attached in parallel" || fail "same name attached in parallel: $(cat "$work/parallel.out" "$work/parallel.err" | tr '\n' '|')"
guest d6 "hdiutil detach -quiet '/Volumes/Unmount Test 1'; hdiutil detach -quiet '/Volumes/Unmount Test'"

# -plist escapes the mount point (macOS: <string>/Volumes/Tom &amp; Jerry &lt;1&gt;</string>)
guest amp "hdiutil attach -plist '$gw/amp-name.dmg'"
grep -q '<string>/Volumes/Tom &amp; Jerry &lt;1&gt;</string>' "$work/amp.out" && pass "-plist escapes the mount point" \
	|| fail "-plist escaping: $(grep -A1 mount-point "$work/amp.out")"
guest d7 "hdiutil detach -quiet '/Volumes/Tom & Jerry <1>'"

# when the file system cannot be mounted, the reason (from FUSE) is reported. fusermount needs write access to the
# mount point in non-root mode; root and rootless mode mount directly, so the case is skipped there.
mkdir "$work/readonly"
chmod 0555 "$work/readonly"
guest fusefail "hdiutil attach -mountpoint '$gw/readonly' '$image'"
if [ "$(rc fusefail)" = 0 ]; then
	echo "SKIP: mount failure message (the mount succeeded: only non-root mode needs write access)"
	guest d8 "hdiutil detach -quiet '$gw/readonly'"
else
	[ "$(rc fusefail)" = 1 ] && [ "$(sed -n 1p "$work/fusefail.err")" = "hdiutil: attach failed - Input/output error" ] \
		&& grep -qi "fusermount" "$work/fusefail.err" && pass "mount failure reports why" \
		|| fail "mount failure message: rc $(rc fusefail), stderr: $(cat "$work/fusefail.err" | tr '\n' '|')"
fi
[ -d "$work/readonly" ] && chmod 0755 "$work/readonly"

# under another mount root the suffix for a taken name starts at 2
mkdir "$work/root2"
guest mr1 "hdiutil attach -mountroot '$gw/root2' '$image'"
guest mr2 "hdiutil attach -mountroot '$gw/root2' '$second'"
[ "$(mount_point mr1)" = "$gw/root2/Unmount Test" ] && [ "$(mount_point mr2)" = "$gw/root2/Unmount Test 2" ] \
	&& pass "same name under -mountroot" || fail "same name under -mountroot: $(cat "$work/mr1.out" "$work/mr1.err" "$work/mr2.out" "$work/mr2.err")"
# a volume outside the guest's /Volumes is not one of DiskArbitration's disks (getfsstat does not list it); detach
# still unmounts it
guest d5 "hdiutil detach '$gw/root2/Unmount Test 2' && hdiutil detach -quiet '$gw/root2/Unmount Test' && test ! -e '$gw/root2/Unmount Test/hello.txt' && test ! -e '$gw/root2/Unmount Test 2/hello.txt'"
[ "$(rc d5)" = 0 ] && [ "$(cat "$work/d5.out")" = "\"$gw/root2/Unmount Test 2\" ejected." ] \
	&& pass "detach under -mountroot" || fail "detach under -mountroot: rc $(rc d5), out: $(cat "$work/d5.out" "$work/d5.err")"

# the attach table, -plist entries and detach through DiskArbitration (macOS 26.7.3: see dmg-attach.md)
"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/da-disks" "$here/da-disks.c" \
	-framework DiskArbitration -framework CoreFoundation >/dev/null 2>&1 || fail "da-disks does not build"
"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/da-ops" "$here/da-ops.c" \
	-framework DiskArbitration -framework CoreFoundation >/dev/null 2>&1 || fail "da-ops does not build"
cp "$here/fixtures/apm.dmg" "$here/fixtures/nolayout.dmg" "$work/"

guest table "hdiutil attach '$image' && '$gw/da-disks' --name '/Volumes/Unmount Test' >'$gw/leaf.out'"
leaf=$(cat "$work/leaf.out" 2>/dev/null)
whole=${leaf%s1}
whole_line=$(tail -n 2 "$work/table.out" | head -n 1)
leaf_line=$(tail -n 1 "$work/table.out")
widths=$(tail -n 2 "$work/table.out" | awk -F '\t' '{ printf "%d %d ", length($1), length($2) }')
[ "$(rc table)" = 0 ] && [ -n "$leaf" ] \
	&& printf '%s\n' "$whole_line" | grep -Eqx "/dev/$whole {0,10}	GUID_partition_scheme {10}	" \
	&& printf '%s\n' "$leaf_line" | grep -Eqx "/dev/$leaf {0,8}	Apple_HFS {22}	/Volumes/Unmount Test" \
	&& [ "$widths" = "20 31 20 31 " ] \
	&& pass "attach table" || fail "attach table: rc $(rc table), disk '$leaf', widths $widths, out: $(cat "$work/table.out" | tr '\t\n' '>|')"
guest dt1 "hdiutil detach -quiet '/Volumes/Unmount Test'"

guest plistdev "hdiutil attach -plist '$image' && '$gw/da-disks' --name '/Volumes/Unmount Test' >'$gw/leaf.out'"
leaf=$(cat "$work/leaf.out" 2>/dev/null)
entries=$(grep -A1 '<key>dev-entry</key>' "$work/plistdev.out" | sed -n 's:.*<string>\(.*\)</string>.*:\1:p' | tr '\n' ' ')
mountable=$(grep -A1 '<key>potentially-mountable</key>' "$work/plistdev.out" | grep -o 'true\|false' | tr '\n' ' ')
[ "$(rc plistdev)" = 0 ] && [ -n "$leaf" ] && [ "$entries" = "/dev/$leaf /dev/${leaf%s1} " ] && [ "$mountable" = "true false " ] \
	&& pass "-plist dev entries" || fail "-plist dev entries: disk '$leaf', entries '$entries', mountable '$mountable'"
guest dt2 "hdiutil detach -quiet '/Volumes/Unmount Test'"

guest apm "hdiutil attach '$gw/apm.dmg'"
[ "$(rc apm)" = 0 ] && [ "$(tail -n 2 "$work/apm.out" | head -n 1 | cut -f 2 | sed 's/ *$//')" = Apple_partition_scheme ] \
	&& [ "$(mount_point apm)" = /Volumes/APMv ] \
	&& pass "apm scheme" || fail "apm scheme: rc $(rc apm), out: $(cat "$work/apm.out" "$work/apm.err" | tr '\t\n' '>|')"
guest dt3 "hdiutil detach -quiet /Volumes/APMv"

guest nolayout "hdiutil attach '$gw/nolayout.dmg'"
[ "$(rc nolayout)" = 0 ] && [ "$(tail -n 2 "$work/nolayout.out" | head -n 1 | cut -f 2 | tr -d ' ')" = "" ] \
	&& [ "$(mount_point nolayout)" = /Volumes/NoLayout ] \
	&& pass "no partition map" || fail "no partition map: rc $(rc nolayout), out: $(cat "$work/nolayout.out" "$work/nolayout.err" | tr '\t\n' '>|')"
guest dt4 "hdiutil detach -quiet /Volumes/NoLayout"

guest bydev "hdiutil attach -quiet '$image' && d=\$('$gw/da-disks' --name '/Volumes/Unmount Test') && echo \${d%s1} >'$gw/whole.out' && hdiutil detach /dev/\${d%s1}"
whole=$(cat "$work/whole.out" 2>/dev/null)
[ "$(rc bydev)" = 0 ] && [ -n "$whole" ] && [ "$(cat "$work/bydev.out")" = "\"$whole\" ejected." ] \
	&& pass "detach by device" || fail "detach by device: rc $(rc bydev), out: $(cat "$work/bydev.out" "$work/bydev.err")"

guest unknowndev "hdiutil detach /dev/disk99"
[ "$(rc unknowndev)" = 1 ] && [ "$(cat "$work/unknowndev.err")" = "hdiutil: detach failed - No such file or directory" ] \
	&& pass "detach unknown device" || fail "detach unknown device: rc $(rc unknowndev), err: $(cat "$work/unknowndev.err")"

guest dissent "(hdiutil attach -quiet '$image' && d=\$('$gw/da-disks' --name '/Volumes/Unmount Test') && echo \${d%s1} >'$gw/whole.out' || exit 9
	'$gw/da-ops' deny 30 >/dev/null & deny=\$!; sleep 2
	hdiutil detach /dev/\${d%s1} >'$gw/dissent1.out' 2>'$gw/dissent1.err'; echo \$? >'$gw/dissent1.rc'
	hdiutil detach -force /dev/\${d%s1} >'$gw/dissent2.out' 2>'$gw/dissent2.err'; echo \$? >'$gw/dissent2.rc'
	test -e '/Volumes/Unmount Test' && echo mounted >'$gw/dissent2.mp' || echo gone >'$gw/dissent2.mp'
	kill \$deny; wait \$deny; hdiutil detach -quiet /dev/\${d%s1})"
whole=$(cat "$work/whole.out" 2>/dev/null)
[ "$(rc dissent1)" = 2 ] && [ "$(cat "$work/dissent1.out")" = "in use by test" ] && [ "$(wc -c < "$work/dissent1.out")" = 14 ] \
	&& [ "$(cat "$work/dissent1.err")" = "hdiutil: couldn't unmount \"$whole\" - Resource busy" ] \
	&& pass "detach with a dissenter" || fail "detach with a dissenter: rc $(rc dissent1), out: $(cat "$work/dissent1.out" "$work/dissent1.err")"
[ "$(rc dissent2)" = 2 ] && [ "$(cat "$work/dissent2.err")" = "hdiutil: couldn't eject \"$whole\" - Resource busy" ] \
	&& [ "$(cat "$work/dissent2.mp")" = gone ] \
	&& pass "-force with a dissenter" || fail "-force with a dissenter: rc $(rc dissent2), mount point $(cat "$work/dissent2.mp"), out: $(cat "$work/dissent2.out" "$work/dissent2.err")"

# the library API
"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/dmg-api" "$here/dmg-api.c" \
	-I"$here/../src/external/darling-dmg/include" -L"$DARLING_ROOT/usr/lib/darling" -ldarling-dmg \
	&& { guest api "'$gw/dmg-api' '$renamed' 'Unmount Test' '$gw/text.txt' '$gw/mountroot'"; cat "$work/api.out" "$work/api.err"
	     [ "$(rc api)" = 0 ] && pass "library API" || fail "library API"; } \
	|| fail "library API test does not build"

# damaged images
"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/dmg-crafted" "$here/dmg-crafted.c" \
	-I"$here/../src/external/darling-dmg/include" -L"$DARLING_ROOT/usr/lib/darling" -ldarling-dmg \
	&& { guest crafted "'$gw/dmg-crafted' '$gw/long-name-key.hfs' '$gw/bad-blkx.dmg'"; cat "$work/crafted.out" "$work/crafted.err"
	     [ "$(rc crafted)" = 0 ] && pass "damaged images" || fail "damaged images: rc $(rc crafted)"; } \
	|| fail "damaged image test does not build"

"$here/unmount-disk-image.sh" || failures=$((failures + 1))

rm -rf "$work"
[ "$failures" = 0 ]
