#!/bin/sh
# A DiskArbitration session keeps getting disk callbacks after diskarbitrationd restarts (da-callbacks.c restart).
# Needs DARLING_SKILLS (a darling-skills checkout) and a running Darling.
#
# Known to fail: when a launchd service restarts, libxpc reconnects an existing connection to it, but the new
# instance never gets the reconnected client (no peer appears, and no request is answered), while a new connection
# to the same service works. The session's own part (it says hello again and catches up) is in DASessionEvents.c.
set -u

here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
gw="/Volumes/SystemRoot$work"

"$DARLING_SKILLS/darling-guest-testing/scripts/guest-cc.sh" -o "$work/da-callbacks" "$here/da-callbacks.c" \
	-framework DiskArbitration -framework CoreFoundation >"$work/build.log" 2>&1 \
	|| { echo "FAIL: da-callbacks does not build: $(cat "$work/build.log")"; rm -rf "$work"; exit 1; }
cp "$here/fixtures/unmount-test.dmg" "$work/"

timeout -s KILL 60 darling shell /bin/sh -c "'$gw/da-callbacks' restart '$gw/unmount-test.dmg' '/Volumes/Unmount Test' \
	'launchctl stop com.apple.diskarbitrationd' >'$gw/out' 2>&1; echo \$? >'$gw/rc'" </dev/null >/dev/null 2>&1
rc=$(cat "$work/rc" 2>/dev/null)
timeout -s KILL 60 darling shell /bin/sh -c "hdiutil detach -quiet '/Volumes/Unmount Test'" </dev/null >/dev/null 2>&1

if [ "$rc" = 0 ]; then
	echo "PASS: callbacks after a daemon restart"
else
	echo "FAIL: callbacks after a daemon restart: rc '$rc' $(tr '\n' '|' < "$work/out" 2>/dev/null)"
fi
rm -rf "$work"
[ "$rc" = 0 ]
