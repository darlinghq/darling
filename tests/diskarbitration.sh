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

# disk appeared, disappeared and description-changed callbacks (see da-callbacks.c)
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

	# a session keeps working after the daemon restarts (launchd starts it again): the session's connection is
	# interrupted, and it says hello again. The daemon is killed with SIGKILL, which nothing can delay.
	cat >"$work/daemon-pid" <<'SCRIPT'
#!/bin/sh
launchctl list | awk '/com.apple.diskarbitrationd$/ { print $1 }'
SCRIPT
	cat >"$work/kill-daemon" <<'SCRIPT'
#!/bin/sh
kill -KILL "$("$(dirname "$0")/daemon-pid")"
SCRIPT
	chmod +x "$work/daemon-pid" "$work/kill-daemon"
	guest cb-restart "(before=\$('$gw/daemon-pid')
		'$gw/da-callbacks' restart '$gw/unmount-test.dmg' '/Volumes/Unmount Test' '$gw/kill-daemon' || exit 1
		after=\$('$gw/daemon-pid')
		test \"\$before\" != \"\$after\" || { echo \"the daemon did not restart (pid \$before)\"; exit 1; })"
	[ "$(rc cb-restart)" = 0 ] && pass "callbacks: after a daemon restart" \
		|| fail "callbacks: after a daemon restart: $(rc cb-restart) $(outputs cb-restart)"
fi

# unmount, eject, mount, approvals and dissenters (see da-ops.c and da-ops-sequence.sh); the values are macOS's,
# except "mount unmounted leaf" (macOS mounts it again; Darling cannot) and the timing cases
if build da-ops "$here/da-ops.c"; then
	cp "$here/da-ops-sequence.sh" "$work/"
	guest ops "'$gw/da-ops-sequence.sh' '$gw'"
	expect() { # <case> <expected result, an extended regular expression>
		got=$(grep "^$1: " "$work/ops.out" | head -n 1 | cut -d: -f2- | sed 's/^ //')
		echo "$got" | grep -Eqx "$2" && pass "ops: $1" || fail "ops: $1: '$got', expected '$2'"
	}
	expect "eject leaf" "status f8da000c"
	expect "unmount unknown leaf" "status f8da0003"
	expect "eject whole while mounted" "status c010"
	expect "unmount with a busy file" "status c010"
	expect "mount mounted leaf" "status f8da0002"
	expect "unmount, dissenter" "status f8da0002 string in use by test"
	expect "whole unmount, dissenter" "status f8da0002 string in use by test"
	expect "eject, dissenter" "status f8da0002 string in use by test"
	expect "forced unmount, dissenter" "status 0"
	expect "mount point after unmount" "gone"
	expect "leaf approval asked" "yes"
	expect "whole approval asked" "yes"
	expect "unmount again" "status f8da0007"
	expect "mount unmounted leaf" "status f8da000c"
	expect "whole unmount, nothing mounted" "status 0"
	expect "eject" "status 0"
	expect "eject again" "status f8da0003"
	expect "unmount, hanging approver" "status 0 in ([0-9]|1[0-5]) s"
	expect "unmount, killed approver" "status 0 in [0-3] s"
	expect "unmount, volume vanished" "status f8da000[37] in ([0-9]|1[0-5]) s"
	expect "disk after vanishing" "absent"
	[ "$(rc ops)" = 0 ] || fail "ops: the sequence failed: $(outputs ops)"
fi

# a FUSE file system whose source is a file, but which is not a disk image (it names no volume kind), is not a disk.
# It is mounted from the host, in the guest's /Volumes; only non-root mode's guest sees host mounts.
if [ "${DARLING_NONROOT:-}" = 1 ] && cc -o "$work/fuse-file-source" "$here/fuse-file-source-host.c" \
		$(pkg-config --cflags --libs fuse3) 2>"$work/ffs.build"; then
	touch "$work/source.img"
	mkdir "$DPREFIX/Volumes/File Source"
	"$work/fuse-file-source" "$work/source.img" "$DPREFIX/Volumes/File Source" &
	sleep 1
	guest filesource "echo \"disk: \$('$gw/da-disks' --name '/Volumes/File Source')\"; df '/Volumes/File Source' | grep -c source.img"
	fusermount3 -u "$DPREFIX/Volumes/File Source"
	wait
	rmdir "$DPREFIX/Volumes/File Source" 2>/dev/null # unless diskarbitrationd removed it
	[ "$(tr '\n' '|' < "$work/filesource.out")" = "disk: |1|" ] \
		&& pass "a FUSE volume that is not a disk image" || fail "a FUSE volume that is not a disk image: $(outputs filesource)"
else
	echo "SKIP: a FUSE volume that is not a disk image (needs non-root mode and libfuse3)"
fi

# only root and the user who attached an image may unmount or eject it: another user gets kDAReturnNotPrivileged.
# Changing the uid needs a guest running as root (rootless mode); elsewhere the case is skipped.
if [ -x "$work/da-ops" ]; then
	guest priv "(hdiutil attach -quiet '$gw/unmount-test.dmg' || exit 9
		leaf=\$('$gw/da-disks' --name '/Volumes/Unmount Test')
		'$gw/da-ops' --uid 501 unmount \$leaf 0; '$gw/da-ops' --uid 501 eject \${leaf%s1}
		test -e '/Volumes/Unmount Test/hello.txt' && echo still-mounted
		hdiutil detach -quiet '/Volumes/Unmount Test')"
	if grep -q "cannot change uid" "$work/priv.out"; then
		echo "SKIP: unmount and eject by another user (the guest cannot change its uid)"
	else
		[ "$(tr '\n' '|' < "$work/priv.out")" = "status f8da0009|status f8da0009|still-mounted|" ] \
			&& pass "unmount and eject by another user" || fail "unmount and eject by another user: $(outputs priv)"
	fi
fi

# without the daemon (its job unloaded) a session still works and knows no disks. Unloading needs a guest running as
# root (rootless mode); elsewhere the case is skipped.
if [ -x "$work/da-ops" ]; then
	guest nodaemon "(P=/System/Library/LaunchDaemons/com.apple.diskarbitrationd.plist
		launchctl unload \$P 2>/dev/null
		if launchctl list | grep -q 'com.apple.diskarbitrationd\$'; then echo not-unloaded; exit 0; fi
		'$gw/da-ops' no-daemon disk0s1; r=\$?
		launchctl load \$P 2>/dev/null; exit \$r)"
	if grep -q not-unloaded "$work/nodaemon.out"; then
		echo "SKIP: session without the daemon (the guest cannot unload the daemon's job)"
	else
		[ "$(rc nodaemon)" = 0 ] && [ "$(grep -v '^status\|^string' "$work/nodaemon.out" | tr '\n' '|')" = "root volume: none|description: absent|released|" ] \
			&& grep -Eq '^status [1-9a-f]' "$work/nodaemon.out" \
			&& pass "session without the daemon" || fail "session without the daemon: rc $(rc nodaemon), $(outputs nodaemon)"
	fi
fi

rm -rf "$work"
[ "$failures" = 0 ]
