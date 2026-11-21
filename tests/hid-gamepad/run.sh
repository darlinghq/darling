#!/bin/bash
# Runs the guest test hid_gamepad_manager against a virtual game controller.
#
# usage: run-hid.sh <guest test binary> <xbox|ds4>
#
# The test (inside Darling) prints "SYNC: <tag>" lines and waits for the file
# <controldir>/go-<tag>; this script (on the host) creates the virtual
# controller with vpad (built from vpad.c, writing /dev/uinput, which needs
# permission), feeds it the events of each step and touches the go file.
# The host user must be able to read the new /dev/input/event node.
set -u
BIN=$1; KIND=${2:-xbox}
HERE=$(cd "$(dirname "$0")" && pwd)
VPAD=${VPAD:-$HERE/vpad}
[ -x "$VPAD" ] || cc -O1 -o "$VPAD" "$HERE/vpad.c" || exit 2
CT=$(mktemp -d); OUT=$CT/out.txt; FIFO=$CT/events
mkfifo "$FIFO"; exec 9<>"$FIFO"

darling shell "$BIN" "/Volumes/SystemRoot$CT" "$KIND" </dev/null > "$OUT" 2>&1 &
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
		plug) "$VPAD" "$KIND" < "$FIFO" > "$CT/vpad.out" 2>&1 & VP=$!; sleep 1;;
		press) ev "key 0x130 1"; ev sync;;
		release) ev "key 0x130 0"; ev sync;;
		stick) [ "$KIND" = xbox ] && ev "abs 0 32767" || ev "abs 0 255"; ev sync;;
		stick-low) [ "$KIND" = xbox ] && ev "abs 0 -32768" || ev "abs 0 0"; ev sync;;
		trigger) [ "$KIND" = xbox ] && ev "abs 2 1023" || ev "abs 5 255"; ev sync;;
		hat-up) ev "abs 0x11 -1"; ev sync;;
		hat-upright) ev "abs 0x10 1"; ev sync;;
		hat-left) ev "abs 0x11 0"; ev "abs 0x10 -1"; ev sync;;
		hat-center) ev "abs 0x10 0"; ev sync;;
		unplug) ev quit; sleep 0.3; kill $VP 2>/dev/null; VP=;;
	esac
	touch "$CT/go-$tag"
done
wait $TP; rc=$?
[ -n "$VP" ] && { ev quit; sleep 0.3; kill $VP 2>/dev/null; }
cat "$OUT"
rm -rf "$CT"
exit $rc
