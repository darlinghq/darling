#!/bin/bash
# Finder.app: opening folders and volumes with LaunchServices (open in the guest), its windows, and quitting after the
# last one closes. See finder.md.
# Needs DARLING_SKILLS (a darling-skills checkout), DARLING_ROOT (the installed tree), and a display of your own,
# exported before the Darling container starts (it keeps the environment it started with; run `darling shutdown`
# first): X11 with DISPLAY (and xdotool), or Wayland with WAYLAND_DISPLAY, XDG_RUNTIME_DIR, SWAYSOCK and
# DARLING_APPKIT_BACKEND=wayland (and swaymsg, wtype). The test types keys into Finder's windows.
# Prints PASS/FAIL lines and exits non-zero if anything failed.
set -u

here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
gw="/Volumes/SystemRoot$work"
failures=0
kb=
trap 'kill $kb 2>/dev/null; chmod -R u+rwx "$work" 2>/dev/null; rm -rf "$work"' EXIT

pass() { echo "PASS: $1"; }
fail() { echo "FAIL: $1"; failures=$((failures + 1)); }
skip() { echo "SKIP: $1"; }

if [ -n "${WAYLAND_DISPLAY:-}" ] && [ "${DARLING_APPKIT_BACKEND:-}" = wayland ]; then
	display=wayland
	# A long-lived virtual keyboard first: a transient one leaves the seat without the keyboard capability.
	wtype -k Shift_L -s 600000 -k Shift_L &
	kb=$!
elif [ -n "${DISPLAY:-}" ]; then
	display=x11
else
	echo "set DISPLAY, or WAYLAND_DISPLAY and DARLING_APPKIT_BACKEND=wayland, to a display of your own" >&2
	exit 2
fi

# runs a shell command line in the guest; stdout, stderr and exit status go to $work/<name>.{out,err,rc}
guest() {
	name=$1
	shift
	timeout -s KILL 300 darling shell /bin/sh -c "{ $1; } >'$gw/$name.out' 2>'$gw/$name.err'; echo \$? >'$gw/$name.rc'" </dev/null >/dev/null 2>&1
}
rc() { cat "$work/$1.rc" 2>/dev/null; }
outputs() { cat "$work/$1.out" "$work/$1.err" 2>/dev/null | tr '\n' '|'; }

running() { guest running "ps ax | grep -c '[F]inder.app/Contents/MacOS/Finder'"; cat "$work/running.out"; }
wait_gone() { # <seconds>: until Finder is not running; 0 if it went
	local i
	for ((i = 0; i < $1; i++)); do [ "$(running)" = 0 ] && return 0; sleep 1; done
	return 1
}
quit_finder() {
	guest quit "for p in \$(ps ax | awk '/[F]inder.app\\/Contents\\/MacOS\\/Finder/ { print \$1 }'); do kill \$p; done; true"
	wait_gone 15
}
volume="/Volumes/Unmount Test"
detach_all() { guest detach-all "for d in '$volume'*; do hdiutil detach -quiet \"\$d\"; done; true"; }

# Finder's windows with that title, one id per line (empty title: every Finder window but the menu bar)
find_windows() {
	if [ $display = x11 ]; then
		xdotool search --onlyvisible --class Finder 2>/dev/null | while read -r w; do
			xprop -id "$w" _NET_WM_WINDOW_TYPE 2>/dev/null | grep -q _DOCK && continue
			[ -z "$1" ] || [ "$(xdotool getwindowname "$w")" = "$1" ] && echo "$w"; done
	else
		swaymsg -t get_tree | python3 -I -c '
import json, sys
title = sys.argv[1]
def walk(n):
    if n.get("app_id") == "Finder" and n.get("type") in ("con", "floating_con") and (not title or n.get("name") == title):
        print(n["id"])
    for c in n.get("nodes", []) + n.get("floating_nodes", []):
        walk(c)
walk(json.load(sys.stdin))' "$1"
	fi
}
windows() { find_windows "$1" | grep -c .; }
window() { # <title> <seconds>: waits for a Finder window with that title; its id in WID; 0 if there is one
	local i
	for ((i = 0; i < $2; i++)); do
		WID=$(find_windows "$1" | tail -1)
		[ -n "$WID" ] && return 0
		sleep 1
	done
	return 1
}
window_gone() { # <title> <seconds>: until no Finder window has that title
	local i
	for ((i = 0; i < $2; i++)); do [ "$(windows "$1")" = 0 ] && return 0; sleep 1; done
	return 1
}
key() { # <key>: to the window window() found, once it has had time to become key; cmd+<key> is Command-<key>
	sleep 2
	if [ $display = x11 ]; then
		xdotool windowactivate --sync "$WID" 2>/dev/null || xdotool windowfocus "$WID"
		sleep 0.5
		case $1 in
			cmd+*) xdotool key "super+${1#cmd+}" ;;
			*) xdotool key "$1" ;;
		esac
	else
		swaymsg -q "[con_id=$WID] focus"
		sleep 0.5
		case $1 in
			cmd+*) wtype -M logo -k "${1#cmd+}" -m logo ;;
			*) wtype -k "$1" ;;
		esac
	fi
	sleep 1
}

# The folders
mkdir -p "$work/F/sub" "$work/G"
touch "$work/F/one.txt" "$work/F/two.txt" "$work/G/three.txt"
mkdir -p "$work/locked"
chmod 000 "$work/locked"
cp "$here/fixtures/unmount-test.dmg" "$work/"
# Finder runs without HOME when LaunchServices launches it, so its home folder is the user database's
guest whoami "id -u; awk -F: -v u=\$(id -u) '\$3 == u { print \$6 }' /etc/passwd"
guest_uid=$(head -1 "$work/whoami.out")
home_name=$(basename "$(sed -n 2p "$work/whoami.out")")
quit_finder
detach_all

# open folder: a window titled with the folder's name
guest open-f "open '$gw/F'"
if [ "$(rc open-f)" = 0 ] && window F 30; then pass "open folder"; else fail "open folder: rc $(rc open-f) $(outputs open-f)"; fi

# open twice: still one window for the folder
guest open-f2 "open '$gw/F'"
sleep 3
n=$(windows F)
if [ "$n" = 1 ]; then pass "open twice"; else fail "open twice: $n windows titled F"; fi

# quit: closing the last window (Command-W) ends Finder
if window F 5; then
	key cmd+w
	if window_gone F 10 && wait_gone 10; then pass "quit"; else fail "quit: windows $(windows F), running $(running)"; fi
else
	fail "quit: no window F"
fi
quit_finder

# launch: launched without documents, Finder shows the home folder
guest launch "open -a Finder"
if [ "$(rc launch)" = 0 ] && window "$home_name" 30; then pass "launch"; else fail "launch: rc $(rc launch) $(outputs launch), windows: $(windows '')"; fi
quit_finder

# volume: DiskImageMounter opens the image's volume, which LaunchServices hands to Finder
guest open-dmg "open '$gw/unmount-test.dmg'"
if window "Unmount Test" 60; then pass "volume"; else fail "volume: $(outputs open-dmg), windows: $(windows '')"; fi
quit_finder
detach_all

# unreadable: a folder that can't be read gives an alert, no window; after it Finder quits
if [ "$guest_uid" = 0 ]; then
	skip "unreadable: the guest runs as root, every folder is readable"
else
	guest open-locked "open '$gw/locked'"
	sleep 5
	all=$(windows '')
	n=$(windows locked)
	if [ "$all" -ge 1 ] && [ "$n" = 0 ] && window '' 1; then
		key Return
		if wait_gone 15; then pass "unreadable"; else fail "unreadable: Finder still runs after the alert"; fi
	else
		fail "unreadable: $all windows, $n titled locked"
	fi
fi
quit_finder

[ $failures = 0 ]
