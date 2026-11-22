#!/bin/bash
# Runs the guest tests of the HID devices (hid_gamepad_manager, hid_keyboard_mouse_output) against a virtual device.
#
# usage: run-hid.sh <guest test binary> <xbox|xboxbt|ds4|xboxff|ds4ff|ds4usbff|kbd|mouse>
#
# The test (inside Darling) prints "SYNC: <tag>" lines and waits for the file
# <controldir>/go-<tag>; this script (on the host) creates the virtual
# controller with vpad (built from vpad.c, writing /dev/uinput, which needs
# permission), feeds it the events of each step and touches the go file.
# The host user must be able to read and write the new /dev/input/event node
# (writing is for LEDs and force feedback). Game controllers usually get that
# from udev; for a keyboard or mouse set HID_NODE_HOOK to a command that
# is run with the path of the new node and grants access (for example a
# script that calls sudo chmod). HID_GUEST_WRAP, if set, is a command prefix for
# the guest run (for example a systemd-run scope with a memory limit).
set -u
BIN=$1; KIND=${2:-xbox}
HERE=$(cd "$(dirname "$0")" && pwd)
VPAD=${VPAD:-$HERE/vpad}
[ -x "$VPAD" ] || cc -O1 -o "$VPAD" "$HERE/vpad.c" || exit 2
CT=$(mktemp -d); OUT=$CT/out.txt; FIFO=$CT/events
mkfifo "$FIFO"; exec 9<>"$FIFO"

${HID_GUEST_WRAP:-} darling shell "$BIN" "/Volumes/SystemRoot$CT" "$KIND" </dev/null > "$OUT" 2>&1 &
TP=$!
VP=
ev() { echo "$@" >&9; }
n=0
while kill -0 $TP 2>/dev/null; do
	sleep 0.2
	tag=$(grep -a "^SYNC: " "$OUT" | sed -n "$((n + 1))p" | cut -d" " -f2)
	[ -z "$tag" ] && continue
	n=$((n + 1))
	case $tag in
		plug) "$VPAD" "$KIND" < "$FIFO" > "$CT/vpad.out" 2>&1 & VP=$!; sleep 1
			if [ -n "${HID_NODE_HOOK:-}" ]; then
				for d in /sys/class/input/event*; do
					grep -q '^Vpad' "$d/device/name" && "$HID_NODE_HOOK" "/dev/input/$(basename "$d")"
				done
			fi;;
		press) ev "key 0x130 1"; ev sync;;
		release) ev "key 0x130 0"; ev sync;;
		stick) case $KIND in xbox*) ev "abs 0 32767";; *) ev "abs 0 255";; esac; ev sync;;
		stick-low) case $KIND in xbox*) ev "abs 0 -32768";; *) ev "abs 0 0";; esac; ev sync;;
		trigger) case $KIND in xbox*) ev "abs 2 1023";; *) ev "abs 5 255";; esac; ev sync;;
		hat-up) ev "abs 0x11 -1"; ev sync;;
		hat-upright) ev "abs 0x10 1"; ev sync;;
		hat-left) ev "abs 0x11 0"; ev "abs 0x10 -1"; ev sync;;
		hat-center) ev "abs 0x10 0"; ev sync;;
		key-a) ev "key 30 1"; ev sync;;
		key-a-up) ev "key 30 0"; ev sync;;
		shift) ev "key 42 1"; ev sync;;
		shift-up) ev "key 42 0"; ev sync;;
		left-down) ev "key 0x110 1"; ev sync;;
		left-up) ev "key 0x110 0"; ev sync;;
		right-down) ev "key 0x111 1"; ev sync;;
		right-up) ev "key 0x111 0"; ev sync;;
		move|move-again) ev "rel 0 5"; ev "rel 1 -3"; ev sync;;
		move-x-big) ev "rel 0 -300"; ev sync;;
		wheel) ev "rel 8 -1"; ev sync;;
		hwheel) ev "rel 6 2"; ev sync;;
		two-frames) ev "rel 0 1"; ev sync; ev "rel 0 2"; ev sync;;
		log*) sleep 0.8; cp "$CT/vpad.out" "$CT/vpad.log";;
		unplug) ev quit; sleep 0.3; kill $VP 2>/dev/null; VP=;;
	esac
	touch "$CT/go-$tag"
done
wait $TP; rc=$?
[ -n "$VP" ] && { ev quit; sleep 0.3; kill $VP 2>/dev/null; }
cat "$OUT"
rm -rf "$CT"
exit $rc
